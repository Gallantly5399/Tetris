//
// Created by amemiya7 on 2024/11/15.
//

#include "Utility.h"
#include <iostream>
//TODO::read from config file
uint32_t utility::scoreTypeToInt(ScoreType scoreType){
    if (scoreType == ScoreType::None) return 0;
    else if (scoreType == ScoreType::Single) return 100;
    else if (scoreType == ScoreType::Double) return 300;
    else if (scoreType == ScoreType::Triple) return 500;
    else if (scoreType == ScoreType::Tetris) return 800;
    else if (scoreType == ScoreType::Combo) return 50;
    else if (scoreType == ScoreType::TSpinMiniNoLines) return 100;
    else if (scoreType == ScoreType::TSpinMiniSingle) return 200;
    else if (scoreType == ScoreType::TSpinMiniDouble) return 400;
    else if (scoreType == ScoreType::TSpinNoLines) return 400;
    else if (scoreType == ScoreType::TSpinSingle) return 800;
    else if (scoreType == ScoreType::TSpinDouble) return 1200;
    else if (scoreType == ScoreType::TSpinTriple) return 1600;
    else if (scoreType == ScoreType::SinglePerfectClear) return 800;
    else if (scoreType == ScoreType::DoublePerfectClear) return 1200;
    else if (scoreType == ScoreType::TriplePerfectClear) return 1600;
    else if (scoreType == ScoreType::TetrisPerfectClear) return 2000;
    else if (scoreType == ScoreType::BackToBackTetrisPerfectClear) return 3200;
    else return 0;
}

uint32_t utility::getScore(Grid &grid, const Block &block) {
    ScoreType scoreType = getScoreType(grid, block);
    uint32_t score = 0;
    if (isDifficultScore(scoreType)) {
        if (grid.backToBack) score += 3 * scoreTypeToInt(scoreType) / 2;
        else score += scoreTypeToInt(scoreType);
        grid.backToBack = true;
        grid.comboCount++;
    } else if (scoreType == ScoreType::Single || scoreType == ScoreType::Double || scoreType == ScoreType::Triple) {
        score += scoreTypeToInt(scoreType);
        grid.backToBack = false;
        grid.comboCount++;
    } else {
        score += scoreTypeToInt(scoreType);
        grid.comboCount = 0;
    }
    if (grid.comboCount >= 2) {
        score += (grid.comboCount - 1) * scoreTypeToInt(ScoreType::Combo);
    }
    grid.clearLines();
    return score;
}

bool utility::TSpin(const Grid &grid, const Block &block) {
    if (block.getType() != BlockType::T || block.getLastMovement() != Movement::Rotate) return false;
    const auto &shape = block.getShape();
    auto [startRow, startColumn] = block.getPosition();
    int count = grid.isOccupied(startRow, startColumn) + grid.isOccupied(startRow + 2, startColumn) +
                grid.isOccupied(startRow, startColumn + 2) + grid.isOccupied(startRow + 2, startColumn + 2);
    return count >= 3;
}

bool utility::isDifficultScore(const ScoreType &scoreType) {
    //TODO::read from config file
    if (scoreType == ScoreType::Tetris || scoreType == ScoreType::TSpinMiniSingle ||
        scoreType == ScoreType::TSpinMiniDouble ||
        scoreType == ScoreType::TSpinSingle || scoreType == ScoreType::TSpinDouble ||
        scoreType == ScoreType::TSpinTriple || scoreType == ScoreType::BackToBackTetrisPerfectClear ||
        scoreType == ScoreType::DoublePerfectClear || scoreType == ScoreType::TriplePerfectClear ||
        scoreType == ScoreType::TetrisPerfectClear || scoreType == ScoreType::SinglePerfectClear) {
        return true;
    }
    return false;
}

int utility::scoreForAI(Grid &grid, const Block &block) {
    ScoreType scoreType = getScoreType(grid, block);
    switch (scoreType) {
        case ScoreType::Single:
            return -2;
        case ScoreType::None:
            return 0;
        case ScoreType::Double:
            return -5;
        case ScoreType::Triple:
            return 1;
        case ScoreType::Tetris:
            return 4;
        case ScoreType::TSpinMiniNoLines:
            return -2;
        case ScoreType::TSpinMiniSingle:
            return -1;
        case ScoreType::TSpinMiniDouble:
            return 2;
        case ScoreType::TSpinNoLines:
            return -2;
        case ScoreType::TSpinSingle:
            return -2;
        case ScoreType::TSpinDouble:
            return 2;
        case ScoreType::TSpinTriple:
            return 4;
        case ScoreType::SinglePerfectClear:
            return 2;
        case ScoreType::DoublePerfectClear:
            return 4;
        case ScoreType::TriplePerfectClear:
            return 6;
        case ScoreType::TetrisPerfectClear:
            return 8;
        case ScoreType::BackToBackTetrisPerfectClear:
            return 10;
        case ScoreType::Combo: //should not be here, maybe add log here
            return 0;
    }
}

ScoreType utility::getScoreType(const Grid &grid, const Block &block) {
    bool isSrs = block.getSrs();
    Grid tempGrid = grid;
    int lines = tempGrid.clearLines();
    ScoreType scoreType = ScoreType::None;
    if (tempGrid.empty()) { //perfect clear
        if (lines == 4 && tempGrid.lines() == 0) {
            if (grid.backToBack) scoreType = ScoreType::BackToBackTetrisPerfectClear;
            else scoreType = ScoreType::TetrisPerfectClear;
        } else if (lines == 3 && tempGrid.lines() == 0) {
            scoreType = ScoreType::TriplePerfectClear;
        } else if (lines == 2 && tempGrid.lines() == 0) {
            scoreType = ScoreType::DoublePerfectClear;
        } else if (lines == 1 && tempGrid.lines() == 0) {
            scoreType = ScoreType::SinglePerfectClear;
        }
    } else if (TSpin(grid, block)) {//T-spin
        //T-spin mini
        if (isSrs || block.checkMiniTSpin(grid)) {
            if (lines == 0) scoreType = ScoreType::TSpinMiniNoLines;
            else if (lines == 1) scoreType = ScoreType::TSpinMiniSingle;
            else if (lines == 2) scoreType = ScoreType::TSpinMiniDouble;
            else if (lines == 3) scoreType = ScoreType::TSpinTriple;
        } else {
            //T-spin
            if (lines == 0) scoreType = ScoreType::TSpinNoLines;
            else if (lines == 1) scoreType = ScoreType::TSpinSingle;
            else if (lines == 2) scoreType = ScoreType::TSpinDouble;
            else if (lines == 3) scoreType = ScoreType::TSpinTriple;
        }
    } else {
        if (lines == 1) scoreType = ScoreType::Single;
        else if (lines == 2) scoreType = ScoreType::Double;
        else if (lines == 3) scoreType = ScoreType::Triple;
        else if (lines == 4) scoreType = ScoreType::Tetris;
    }
    return scoreType;
}
