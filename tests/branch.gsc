IF (TRUE) {
    Print("Passed 1");
} ELSE {
    Print("Failed 1");
}

IF (!TRUE) {
    Print("Failed 2");
} ELSE {
    Print("Passed 2");
}

IF (FALSE) {
    Print("Failed 3");
} ELSEIF (TRUE) {
    Print("Passed 3");
}

IF (FALSE) {} ELSEIF (!FALSE) {
    Print("Passed 4");
}

IF (TRUE) {
    IF (TRUE) {
        IF (TRUE) {
            Print("Passed 5");
        } ELSE {
            Print("Failed 5");
        }
        Print("Passed 6");
    } ELSE {
        Print("Failed 6");
    }
    Print("Passed 7");
} ELSE {
    Print("Failed 7");
}
