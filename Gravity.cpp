//
// Created by amemiya7 on 2024/6/20.
//

#include "Gravity.h"
#include <algorithm>

void Gravity::setSoftDrop() {
    level += 5;
    level = std::max(level, 20);
    updateFallTime();
}


void Gravity::reset() {
    this->level = preLevel;
    updateFallTime();
}

void Gravity::updateFallTime() {
    fallTime = std::pow((0.8-((level-1)*0.007)), (level-1));
}

void Gravity::levelUp() {
    this->level ++;
    preLevel = level;
    updateFallTime();
}

double Gravity::getFallTime() const {
    return fallTime;
}

Gravity::Gravity() {
    preLevel = 1;
    level = 1;
    updateFallTime();
}

Gravity::Gravity(int level) {
    preLevel = level;
    this->level = level;
    updateFallTime();
}
