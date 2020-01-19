#include "ISOImageRipper.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <numeric>

#include "crc32.h"

constexpr long long SECTOR_SIZE = 0x800;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;


int ISOImageRipper::open(const TCHAR* filename)
{
	if (m_isofile != nullptr) {
		closefile();
	}

	m_isofile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_isofile == INVALID_HANDLE_VALUE) {
		m_isofile = nullptr;
		return -1;
	}
	
	LARGE_INTEGER filesize;

	if (!GetFileSizeEx(m_isofile, &filesize)) {
		closefile();
		return -1;
	}

	m_filesize = filesize.QuadPart;

	m_headbuf.resize(16 * 1024 * 1024);
	DWORD rsize;
	ReadFile(m_isofile, m_headbuf.data(), static_cast<DWORD>(m_headbuf.size()), &rsize, NULL);
	return 0;
}

int ISOImageRipper::scan()
{
	u8* buf = m_headbuf.data() + 0x8000;

	while (true) {
		if (buf + SECTOR_SIZE > m_headbuf.data() + m_headbuf.size()) {
			return -1;
		}
		if (buf[1] != 'C' || buf[2] != 'D' || buf[3] != '0' || buf[4] != '0' || buf[5] != '1') {
			return -1;
		}
		if (buf[0] == 0xff) {
			return -1;
		}
		if (buf[0] != 0x01) {
			buf += SECTOR_SIZE;
			continue;
		}

		if (buf[156] != 0x22) {
			return -1;
		}

		std::vector<char> fullname(2);
		fullname[0] = '\\';
		fullname[1] = '\0';
		int result = scanDir(*(u32*)(buf + 156 + 2), *(u32*)(buf + 156 + 10), &fullname);
		printf("file count: %d\n", static_cast<int>(m_filecount));
		return result;
	}
}

int ISOImageRipper::scanDir(unsigned long long lba, unsigned long long size, std::vector<char> *fullname)
{
	u8* buf = m_headbuf.data() + lba * SECTOR_SIZE;
	size_t pathsize = fullname->size();
	std::vector<char> filename;
	for (unsigned long long offset = 0; offset < size; offset += buf[offset + 0x0]) {
		if (buf[offset + 0x0] == 0x0) {
			if (offset + SECTOR_SIZE < size) {
				offset = (offset / SECTOR_SIZE + 1) * SECTOR_SIZE;
			}
			else {
				break;
			}
		}

		u64 fileLBA = *(u32*)(buf + offset + 0x2);
		if (fileLBA <= lba) {
			continue;
		}

		u8 isDir = buf[offset + 25] & 0x2;
		u64 filesize = *(u32 *)(buf + offset + 10);

		long long filenamesize = buf[offset + 32];
		if (buf[offset + 33 + filenamesize - 1] == ';') {
			filenamesize -= 2;
		}
		filename.resize(filenamesize + 1);
		memcpy(filename.data(), buf + offset + 33, filenamesize);

		if ((!isDir) && (filename[0] == '_')) {
			filename[0] = '.';
		}
		if ((!isDir) && (filename[filenamesize - 1] == '.')) {
			--filenamesize;
		}
		filename[filenamesize] = '\0';

		if (isDir) {
			fullname->resize(pathsize + filenamesize + 1);
			memcpy(fullname->data() + pathsize - 1, filename.data(), filenamesize);
			fullname->at(pathsize + filenamesize - 1) = '\\';
			fullname->at(pathsize + filenamesize) = '\0';
			std::printf("LBA: %d, Size: %d, ", static_cast<int>(fileLBA), static_cast<int>(filesize));
			std::printf("Name:%s\n", fullname->data());
			scanDir(fileLBA, filesize, fullname);
		}
		else {
			fullname->resize(pathsize + filenamesize);
			memcpy(fullname->data() + pathsize - 1, filename.data(), filenamesize);
			fullname->at(pathsize + filenamesize - 1) = '\0';

			if (memcmp(fullname->data(), "\\PSP_GAME\\SYSDIR\\UPDATE\\", 24) == 0 || memcmp(fullname->data(), "\\PSP_GAME\\SYSDIR\\BOOT.BIN", 25) == 0) {
				*(u64*)(buf + offset + 10) = 0x0;
				filesize = 0;
				std::printf("!!!!File Ripped:%s\n", fullname->data());
			}

			if (filesize != 0) {
				unsigned int crc = crc32(fileLBA, filesize);
				std::printf("LBA: %d, Size: %d, CRC32: %08x, ", static_cast<int>(fileLBA), static_cast<int>(filesize), crc);
				m_filecrc32.push_back(crc);
			}
			else {
				std::printf("Null File. ");
				m_filecrc32.push_back(0);
			}
			m_filerec.push_back(buf - m_headbuf.data() + offset);
			m_filefullname.push_back(*fullname);
			std::printf("Name:%s\n", fullname->data());


			++m_filecount;
		}
		fullname->resize(pathsize);
		fullname->at(pathsize - 1) = '\0';
	}
	return 0; 
}

unsigned int ISOImageRipper::crc32(unsigned long long lba, unsigned long long size)
{
	std::vector<u8> filebuf(size);

	DWORD rsize;
	LARGE_INTEGER offset;
	offset.QuadPart = lba * SECTOR_SIZE;
	SetFilePointerEx(m_isofile, offset, NULL, FILE_BEGIN);
	ReadFile(m_isofile, filebuf.data(), static_cast<DWORD>(size), &rsize, NULL);
	return ::crc32(filebuf.data(), size);
}

int ISOImageRipper::checkDup()
{
	size_t size = m_filecrc32.size();
	std::vector<size_t> orderidx(size);
	std::iota(orderidx.begin(), orderidx.end(), 0);
	std::sort(orderidx.begin(), orderidx.end(), [&](size_t i1, size_t i2) { return getFileLBA(m_filerec[i1]) < getFileLBA(m_filerec[i2]); });
	std::stable_sort(orderidx.begin(), orderidx.end(), [&](size_t i1, size_t i2) { return m_filecrc32[i1] < m_filecrc32[i2]; });

	m_filelink.resize(size);
	std::fill(m_filelink.begin(), m_filelink.end(), -1);
	u32 crc = ~0U;
	long long one = -1;
	for (size_t i = 0; i < size; ++i) {
		if (i < size - 1 && crc != m_filecrc32[orderidx[i]] && m_filecrc32[orderidx[i]] == m_filecrc32[orderidx[i + 1]]) {
			crc = m_filecrc32[orderidx[i]];
			one = orderidx[i];
			printf("crc: %08x\n", crc);
		}
		if (crc == m_filecrc32[orderidx[i]]) {
			printf("%s\n", m_filefullname[orderidx[i]].data());
			if (one != orderidx[i]) {
				m_filelink[orderidx[i]] = one;
			}
		}
	}
	return 0;
}

int ISOImageRipper::write(const TCHAR* filename)
{
	u64 firstfilelba = m_filesize / SECTOR_SIZE;
	size_t size = m_filerec.size();
	for (auto offset : m_filerec) {
		if (getFileLBA(offset) < firstfilelba) {
			firstfilelba = getFileLBA(offset);
		}
	}
	std::vector<u8> newheadbuf(firstfilelba * SECTOR_SIZE);
	memcpy(newheadbuf.data(), m_headbuf.data(), firstfilelba * SECTOR_SIZE);

	std::vector<size_t> orderidx(size);
	std::iota(orderidx.begin(), orderidx.end(), 0);
	std::sort(orderidx.begin(), orderidx.end(), [&](size_t i1, size_t i2) { return getFileLBA(m_filerec[i1]) < getFileLBA(m_filerec[i2]); });

	long long filelba = firstfilelba;
	std::vector<u64> source;
	for (size_t i = 0; i < size; ++i) {
		if (m_filelink[i] == -1) {
			*(u32*)(newheadbuf.data() + m_filerec[i] + 2) = static_cast<u32>(filelba);
			*(u32*)(newheadbuf.data() + m_filerec[i] + 6) = _byteswap_ulong(static_cast<u32>(filelba));
			filelba += getFileSize(m_filerec[i]) / SECTOR_SIZE + ((getFileSize(m_filerec[i]) % SECTOR_SIZE != 0) ? 1 : 0);
			source.push_back(m_filerec[i]);
		}
		else {
			*(u64*)(newheadbuf.data() + m_filerec[i] + 2) = *(u64*)(newheadbuf.data() + m_filerec[m_filelink[i]] + 2);
		}
	}

	HANDLE ripiso = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD bytes_write;
	WriteFile(ripiso, newheadbuf.data(), static_cast<DWORD>(newheadbuf.size()), &bytes_write, NULL);
	for (auto offset : source) {
		std::vector<u8> filebuf;
		u64 size;
		size = getFileSize(offset) / SECTOR_SIZE * SECTOR_SIZE + ((getFileSize(offset) % SECTOR_SIZE != 0) ? SECTOR_SIZE : 0);
		filebuf.resize(size);

		DWORD rsize;
		LARGE_INTEGER fileoffset;
		fileoffset.QuadPart = getFileLBA(offset) * SECTOR_SIZE;
		SetFilePointerEx(m_isofile, fileoffset, NULL, FILE_BEGIN);
		ReadFile(m_isofile, filebuf.data(), static_cast<DWORD>(size), &rsize, NULL);
		WriteFile(ripiso, filebuf.data(), static_cast<DWORD>(rsize), &bytes_write, NULL);
	}

	return 0;
}
