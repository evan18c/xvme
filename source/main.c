// x86 Emulator
// Author: Evan Cassidy
// Date: 4/22/2026

// Xbox
#include "xbox.h"

// Main
int main() {

    Xbox *xbox = XboxNew();
    XboxLoadXBE(xbox, "Default.xbe");
    XboxRun(xbox);

    return 0;
}