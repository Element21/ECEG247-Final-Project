#pragma once

void debouceDelay(void);
void formatTimeString(char* outStr, const char* templateStr, unsigned long tCount, unsigned int startPos);
void formatCount(char* outStr, const char* templateStr, unsigned int v1, unsigned int p1, unsigned int v2, unsigned int p2);
void formatDistance(char* outStr, const char* templateStr, float value, unsigned int startPos);
void formatAccuracy(char* outStr, const char* templateStr, float accuracy, unsigned int startPos);

float randomNumBetween(float min, float max);