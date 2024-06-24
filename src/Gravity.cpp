//
// Created by amemiya7 on 2024/6/20.
//

#include "Gravity.h"
#include <algorithm>

void Gravity::setSoftDrop() {
    isSoftDrop = true;
}


void Gravity::unsetSoftDrop() {
    isSoftDrop = false;
}

void Gravity::updateFallTime() {
    fallTime = std::pow((0.8-((level-1)*0.007)), (level-1));
}

void Gravity::levelUp() {
    level ++;
    level = std::min(level, 19);
    updateFallTime();
}

double Gravity::getFallTime() const {
    if (isSoftDrop) {
        int tempLevel = std::min(level + 5, 19);
        return std::pow((0.8-((tempLevel + 5-1)*0.007)), (tempLevel + 5-1));
    }
    if (noGravity) return std::numeric_limits<double>::max();
    return fallTime;
}

Gravity::Gravity() {
    level = 1;
    updateFallTime();
}

Gravity::Gravity(int level) {
    this->level = level;
    updateFallTime();
}

int Gravity::getLevel() const{
    return level;
}

void Gravity::addLines(int lines) {
    clearedLines += lines;
    if (clearedLines >= 10 * level) {
        levelUp();
    }
}

void Gravity::setNoGravity() {
    noGravity = true;
}

int Gravity::getLines() const {
    return clearedLines;
}

void Gravity::clear() {
    clearedLines = 0;
    level = 1;
    updateFallTime();
    isSoftDrop = false;
}
