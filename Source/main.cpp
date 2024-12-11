#include "ReBit.h"

int main() {
    ReBit app;
    if (app.Construct())
        app.Start();
    return 0;
}
