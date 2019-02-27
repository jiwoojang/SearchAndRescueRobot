#ifndef ENUMS
#define ENUMS
enum MachineStates{
    STARTINGCOURSE = 0,
    INITIALSEARCH = 1,
    LOOKFORFIRE = 2,
    SCANNINGFIRE = 3,
    AWAITINGFIRESCAN = 4,
    STOPPED = 5,
    COUNT = 6// ALWAYS MAKE COUNT LAST, UGLY SOLUTION TO DETERMINE ITEMS IN ENUM
};

enum SearchingStates{
    DRIVETOPOINT = 0,
    GRIDSEARCH = 1
    //COUNT // ALWAYS MAKE COUNT LAST, UGLY SOLUTION TO DETERMINE ITEMS IN ENUM
};
#endif
