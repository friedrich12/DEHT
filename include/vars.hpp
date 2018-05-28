#pragma once

const int LOGSIZE = 8;
const int SIZE = 1<<LOGSIZE;

// successors list size (to continue operating on node failures)
const int N_SUCCESSORS = 4;

// INT = interval in seconds
// RET = retry limit

// Stabilize
const int STABILIZE_INT = 1;
const int STABILIZE_RET = 4;

// Fix Fingers
const int FIX_FINGERS_INT = 4;

// Update Successors
const int UPDATE_SUCCESSORS_INT = 1;
const int UPDATE_SUCCESSORS_RET = 6;

// Find Successors
const int FIND_SUCCESSOR_RET = 3;
const int FIND_PREDECESSOR_RET = 3;
