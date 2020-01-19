#include "ISOImageRipper.h"
#include "FileDialog.h"

int main()
{
    ISOImageRipper iso;
    std::vector<WCHAR> ifile;
    if (getInputFile(&ifile) == 0) {
        return 1;
    }
    std::vector<WCHAR> ofile;
    if (getOutputFile(&ofile) == 0) {
        return 2;
    }
    iso.open(ifile.data());
    iso.scan();
    iso.checkDup();
    iso.write(ofile.data());
    iso.closefile();
    return 0;
}
