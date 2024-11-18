//
// Created by amemiya7 on 2024/6/24.
//

#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include "Generator.h"
#include <toml++/toml.hpp>
#include <filesystem>
#include "Ai.h"

//TODO::modify linesWeight to scoreWeight
struct Candidate {
    int32_t heightWeight;
    int32_t scoreWeight;
    int32_t holesWeight;
    int32_t bumpinessWeight;
    int32_t emptyLinesWeight;
    int32_t backToBackWeight;
    int32_t comboWeight;
    int32_t tetrisWeight;
    int32_t perfectClearWeight;
    int32_t tSpinWeight;
    int32_t singleWeight;
    int32_t doubleWeight;
    int32_t tripleWeight;
    int64_t fitness;
};

class Tuner {
public:
    void output(const Candidate &candidate) {
        std::cout << ",fitness:" << candidate.fitness
                  << "heightWeight:" << candidate.heightWeight
                  << ",linesWeight:" << candidate.scoreWeight
                  << ",holesWeight:" << candidate.holesWeight
                  << ",bumpinessWeight:" << candidate.bumpinessWeight
                  << ",emptyLinesWeight:" << candidate.emptyLinesWeight
                  << ",backToBackWeight:" << candidate.backToBackWeight
                  << ",comboWeight" << candidate.comboWeight
                  << ",tetrisWeight" << candidate.tetrisWeight
                  << ",perfectClearWeight" << candidate.perfectClearWeight
                  << ",tSpinWeight" << candidate.tSpinWeight
                  << ",singleWeight" << candidate.singleWeight
                  << ",doubleWeight" << candidate.doubleWeight
                  << ",tripleWeight" << candidate.tripleWeight
                  << '\n';
    }

    Tuner() {
        std::random_device rd;
        gen = std::mt19937(rd());
        const std::filesystem::path projectPath(FILE_LOCATION);
        auto config = toml::parse_file((projectPath / "config.toml").c_str());
        MAX_THREAD = config["tuner"]["MAX_THREAD"].value_or(1);
    }

    static void normalize(Candidate &candidate) {
        candidate.heightWeight = std::max(1, std::min(1000, candidate.heightWeight));
        candidate.scoreWeight = std::max(1, std::min(1000, candidate.scoreWeight));
        candidate.holesWeight = std::max(1, std::min(1000, candidate.holesWeight));
        candidate.bumpinessWeight = std::max(1, std::min(1000, candidate.bumpinessWeight));
        candidate.emptyLinesWeight = std::max(1, std::min(1000, candidate.emptyLinesWeight));
        candidate.backToBackWeight = std::max(1, std::min(1000, candidate.backToBackWeight));
        candidate.comboWeight = std::max(1, std::min(1000, candidate.comboWeight));
        candidate.tetrisWeight = std::max(1, std::min(1000, candidate.tetrisWeight));
        candidate.perfectClearWeight = std::max(1, std::min(1000, candidate.perfectClearWeight));
        candidate.tSpinWeight = std::max(1, std::min(1000, candidate.tSpinWeight));
        candidate.singleWeight = std::max(1, std::min(1000, candidate.singleWeight));
        candidate.doubleWeight = std::max(1, std::min(1000, candidate.doubleWeight));
        candidate.tripleWeight = std::max(1, std::min(1000, candidate.tripleWeight));
    }

    Candidate generateRandomCandidate() {
        Candidate candidate = {
                .heightWeight = randomInteger(1, 1000),
                .scoreWeight = randomInteger(1, 1000),
                .holesWeight = randomInteger(1, 1000),
                .bumpinessWeight = randomInteger(1, 1000),
                .emptyLinesWeight =randomInteger(1, 1000),
                .backToBackWeight = randomInteger(1, 1000),
                .comboWeight = randomInteger(1, 1000),
                .tetrisWeight = randomInteger(1, 1000),
                .perfectClearWeight = randomInteger(1, 1000),
                .tSpinWeight = randomInteger(1, 1000),
                .singleWeight = randomInteger(1, 1000),
                .doubleWeight = randomInteger(1, 1000),
                .tripleWeight = randomInteger(1, 1000),
        };
//        normalize(candidate);
        return candidate;
    }

    //TODO::add log file
    void computeFitnesses(std::vector<Candidate> &candidates, int numberOfGames, int maxNumberOfMoves) {
        if (MAX_THREAD == 1) {
            for (int i = 0; i < candidates.size(); i++) {
                threadRun(candidates, numberOfGames, maxNumberOfMoves, i);
            }
        } else {
            uint32_t totalThreads = MAX_THREAD;
            std::vector<std::thread> threads(totalThreads - 1);
            int threadTask = candidates.size() / totalThreads;
            for (int i = 0; i < threads.size(); i++) {
                threads[i] = std::thread([i, totalThreads, &candidates, numberOfGames, maxNumberOfMoves, this] {
                    for (int j = i; j < candidates.size(); j += totalThreads) {
                        this->threadRun(candidates, numberOfGames, maxNumberOfMoves, j);
                    }
                });
            }
            for (int i = totalThreads - 1; i < candidates.size(); i += totalThreads) {
                threadRun(candidates, numberOfGames, maxNumberOfMoves, i);
            }
            for (auto &thread: threads) {
                thread.join();
            }
        }
    }


    std::pair<Candidate, Candidate> tournamentSelectPair(const std::vector<Candidate> &candidates, int ways) {
        std::vector<int> indices;
        for (int i = 0; i < candidates.size(); i++) {
            indices.push_back(i);
        }
        std::shuffle(indices.begin(), indices.end(), gen);
        /*
            Note that the following assumes that the candidates array is
            sorted according to the fitness of each individual candidates.
            Hence it suffices to pick the least 2 indexes out of the random
            ones picked.
        */
        int fittestCandidateIndex1 = -1;
        int fittestCandidateIndex2 = -1;
        for (int i = 0; i < ways; i++) {
            int selectedIndex = indices.back();
            indices.pop_back();
            if (fittestCandidateIndex1 == -1 || selectedIndex < fittestCandidateIndex1) {
                fittestCandidateIndex2 = fittestCandidateIndex1;
                fittestCandidateIndex1 = selectedIndex;
            } else if (fittestCandidateIndex2 == -1 || selectedIndex < fittestCandidateIndex2) {
                fittestCandidateIndex2 = selectedIndex;
            }
        }
        return {candidates[fittestCandidateIndex1], candidates[fittestCandidateIndex2]};
    }

    //TODO:: change crossOver
    static Candidate crossOver(const Candidate &candidate1, const Candidate &candidate2) {
        double totalFitness = candidate1.fitness + candidate2.fitness;
        double coefficient1 = 0, coefficient2 = 0;
        if (totalFitness == 0) {
            coefficient1 = coefficient2 = 0.5;
        } else {
            coefficient1 = 1.0 * candidate1.fitness / totalFitness;
            coefficient2 = 1.0 * candidate2.fitness / totalFitness;
        }
        Candidate candidate = {
                .heightWeight = static_cast<int32_t>(coefficient1 * candidate1.heightWeight +
                                                     coefficient2 * candidate2.heightWeight),
                .scoreWeight =  static_cast<int32_t>(coefficient1 * candidate1.scoreWeight +
                                                     coefficient2 * candidate2.scoreWeight),
                .holesWeight = static_cast<int32_t>(coefficient1 * candidate1.holesWeight +
                                                    coefficient2 * candidate2.holesWeight),
                .bumpinessWeight = static_cast<int32_t>(coefficient1 * candidate1.bumpinessWeight +
                                                        coefficient2 * candidate2.bumpinessWeight),
                .emptyLinesWeight = static_cast<int32_t>(coefficient1 * candidate1.emptyLinesWeight +
                                                         coefficient2 * candidate2.emptyLinesWeight),
                .backToBackWeight = static_cast<int32_t>(coefficient1 * candidate1.backToBackWeight +
                                                         coefficient2 * candidate2.backToBackWeight),
                .comboWeight = static_cast<int32_t>(coefficient1 * candidate1.comboWeight +
                                                    coefficient2 * candidate2.comboWeight),
                .tetrisWeight = static_cast<int32_t>(coefficient1 * candidate1.tetrisWeight +
                                                     coefficient2 * candidate2.tetrisWeight),
                .perfectClearWeight = static_cast<int32_t>(coefficient1 * candidate1.perfectClearWeight +
                                                           coefficient2 * candidate2.perfectClearWeight),
                .tSpinWeight = static_cast<int32_t>(coefficient1 * candidate1.tSpinWeight +
                                                    coefficient2 * candidate2.tSpinWeight),
                .singleWeight = static_cast<int32_t>(coefficient1 * candidate1.singleWeight +
                                                     coefficient2 * candidate2.singleWeight),
                .doubleWeight = static_cast<int32_t>(coefficient1 * candidate1.doubleWeight +
                                                     coefficient2 * candidate2.doubleWeight),
                .tripleWeight = static_cast<int32_t>(coefficient1 * candidate1.tripleWeight +
                                                     coefficient2 * candidate2.tripleWeight),


        };
        normalize(candidate);
        return candidate;
    }

    void mutate(Candidate &candidate) {
        auto quantity = randomInteger(-100, 100);
        for (int i = 0; i < 3; i++) {
            switch (randomInteger(0, 12)) {
                case 0:
                    quantity = randomInteger(-candidate.heightWeight + 1, candidate.heightWeight);
                    candidate.heightWeight = std::max(candidate.heightWeight + quantity, 0);
                    break;
                case 1:
                    quantity = randomInteger(-candidate.scoreWeight + 1, candidate.scoreWeight);
                    candidate.scoreWeight = std::max(candidate.scoreWeight + quantity, 0);
                    break;
                case 2:
                    quantity = randomInteger(-candidate.holesWeight + 1, candidate.holesWeight);
                    candidate.holesWeight = std::max(candidate.holesWeight + quantity, 0);
                    break;
                case 3:
                    quantity = randomInteger(-candidate.bumpinessWeight + 1, candidate.bumpinessWeight);
                    candidate.bumpinessWeight = std::max(candidate.bumpinessWeight + quantity, 0);
                    break;
                case 4:
                    quantity = randomInteger(-candidate.emptyLinesWeight + 1, candidate.emptyLinesWeight);
                    candidate.emptyLinesWeight = std::max(candidate.emptyLinesWeight + quantity, 0);
                    break;
                case 5:
                    quantity = randomInteger(-candidate.backToBackWeight + 1, candidate.backToBackWeight);
                    candidate.backToBackWeight = std::max(candidate.backToBackWeight + quantity, 0);
                    break;
                case 6:
                    quantity = randomInteger(-candidate.comboWeight + 1, candidate.comboWeight);
                    candidate.comboWeight = std::max(candidate.comboWeight + quantity, 0);
                    break;
                case 7:
                    quantity = randomInteger(-candidate.tetrisWeight + 1, candidate.tetrisWeight);
                    candidate.tetrisWeight = std::max(candidate.tetrisWeight + quantity, 0);
                    break;
                case 8:
                    quantity = randomInteger(-candidate.perfectClearWeight + 1, candidate.perfectClearWeight);
                    candidate.perfectClearWeight = std::max(candidate.perfectClearWeight + quantity, 0);
                    break;
                case 9:
                    quantity = randomInteger(-candidate.tSpinWeight + 1, candidate.tSpinWeight);
                    candidate.tSpinWeight = std::max(candidate.tSpinWeight + quantity, 0);
                    break;
                case 10:
                    quantity = randomInteger(-candidate.singleWeight + 1, candidate.singleWeight);
                    candidate.singleWeight = std::max(candidate.singleWeight + quantity, 0);
                    break;
                case 11:
                    quantity = randomInteger(-candidate.doubleWeight + 1, candidate.doubleWeight);
                    candidate.doubleWeight = std::max(candidate.doubleWeight + quantity, 0);
                    break;
                case 12:
                    quantity = randomInteger(-candidate.tripleWeight + 1, candidate.tripleWeight);
                    candidate.tripleWeight = std::max(candidate.tripleWeight + quantity, 0);
                    break;
            }
        }
    }

    static void
    deleteNLastReplacement(std::vector<Candidate> &candidates, const std::vector<Candidate> &newCandidates) {
        for (int i = 0; i < newCandidates.size(); i++) candidates.pop_back();
        for (int i = 0; i < newCandidates.size(); i++) {
            candidates.push_back(newCandidates[i]);
        }
        std::sort(candidates.begin(), candidates.end(), [&](Candidate a, Candidate b) {
            return a.fitness > b.fitness;
        });
    }

    /*
        Population size = 100
        Rounds per candidate = 5
        Max moves per round = 200
        Theoretical fitness limit = 5 * 200 * 4 / 10 = 400
    */
    void run() {
        //TODO:: add concurrency
        std::vector<Candidate> candidates;
        // Initial population generation
        for (int i = 0; i < 100; i++) {
            candidates.emplace_back(generateRandomCandidate());
        }
        std::cout << "Computing fitnesses of initial candidates.\n";
        //timer
        computeFitnesses(candidates, 10, 300);
        std::sort(candidates.begin(), candidates.end(), [&](Candidate a, Candidate b) {
            return a.fitness > b.fitness;
        });

        int count = 0;
        for (int _ = 0; _ < 10000; _++) {
            std::vector<Candidate> newCandidates;
            for (int i = 0; i < 30; i++) { // 30% of population
                auto pair = tournamentSelectPair(candidates, 10); // 10% of population
                Candidate candidate = crossOver(pair.first, pair.second);
                if (randomInteger(1, 100) <= 10) {
                    mutate(candidate);
//                    output(candidate);
                }
                normalize(candidate);
                newCandidates.push_back(candidate);
            }
            std::cout << "Computing fitnesses of new candidates. (" + std::to_string(count) + ")\n";
            computeFitnesses(newCandidates, 10, 300);
            std::cout
                    << "Replacing the least 30% of the population with the new candidates. (" + std::to_string(count) +
                       ")\n";
            deleteNLastReplacement(candidates, newCandidates);
            int totalFitness = 0;
            for (int i = 0; i < candidates.size(); i++) {
                totalFitness += candidates[i].fitness;
            }

            //TODO::reflection
            //TODO::write to file
            std::cout << "Average fitness = " << 1.0 * totalFitness / candidates.size() << '\n';
            std::cout << "Highest fitness = " << candidates[0].fitness << "(" << count << ")\n";
            std::cout << "Fittest candidate = " << "heightWeight:" << candidates[0].heightWeight << ',' <<
                      "bumpinessWeight:" << candidates[0].bumpinessWeight << ',' << "holesWeight:"
                      << candidates[0].holesWeight << ',' <<
                      "scoreWeight:" << candidates[0].scoreWeight <<
                      ",emptyLinesWeight:" << candidates[0].emptyLinesWeight <<
                      ",backToBackWeight:" << candidates[0].backToBackWeight <<
                      ",comboWeight:" << candidates[0].comboWeight <<
                      ",tetrisWeight:" << candidates[0].tetrisWeight <<
                        ",perfectClearWeight:" << candidates[0].perfectClearWeight <<
                        ",tSpinWeight:" << candidates[0].tSpinWeight <<
                        ",singleWeight:" << candidates[0].singleWeight <<
                        ",doubleWeight:" << candidates[0].doubleWeight <<
                        ",tripleWeight:" << candidates[0].tripleWeight <<
                      "(" << count << ")\n";
            count++;
        }
    };

private:
    std::mt19937 gen;
    uint32_t MAX_THREAD = 1;
    void threadRun(std::vector<Candidate> &candidates, int numberOfGames, int maxNumberOfMoves, int index) {
        Candidate &candidate = candidates[index];
        AI ai = AI(candidate.heightWeight, candidate.scoreWeight, candidate.holesWeight,
                   candidate.bumpinessWeight, candidate.emptyLinesWeight, candidate.backToBackWeight,
                   candidate.comboWeight, candidate.tetrisWeight, candidate.perfectClearWeight,
                   candidate.tSpinWeight, candidate.singleWeight, candidate.doubleWeight, candidate.tripleWeight);
        uint64_t totalScore = 0;
        int totalMovement = 0;
        for (int j = 0; j < numberOfGames; j++) {
            Grid grid(10, 22);
            Generator rpg{};

            uint64_t score = 0;
            int numberOfMoves = 0;
            while ((numberOfMoves++) < maxNumberOfMoves && !grid.exceed()) {
                Block workingPiece = rpg.nextBlock();
                std::vector<Block> workingPieces = {workingPiece};
                for (const auto &piece: rpg.seeNextBlocks(9)) workingPieces.push_back(piece);
                auto [_nouse, movements, _grid_nouse_1, _dmakd] = ai.best_(grid, workingPieces);
                //TODO::move
                for (auto movement: movements) {
                    if (movement == Movement::Hold) {
                        if (movements.size() != 1) {
                            //TODO::Log
                            std::cerr << "Hold is not the only movement\n";
                        }
                        grid.hold(workingPiece);
                        if (workingPiece.empty()) continue;
                        auto [_score_nouse, secondMovements, _grid_nouse_2, _aisdjas] = ai.best_(grid, workingPieces);
                        for (auto secondMovement: secondMovements) {
                            if (secondMovement == Movement::Hold) { //TODO::Log
                                std::cerr << "Hold is not the only movement\n";
                            }
                            utility::move(grid, workingPiece, secondMovement);
//                            if (!utility::move(grid, workingPiece, secondMovement)) {
////                                std::cerr << "Invalid movement1\n";
////                                exit(-1);
//                                flag = false;
//                                temMovement = movement;
//                            }
                        }
                    } else {
                        utility::move(grid, workingPiece, movement);
//                        if (!utility::move(grid, workingPiece, movement)) {
////                            std::cerr << "Invalid movement2;" << movement << ", ";
////                            exit(-1);
//                                flag = false;
//                                temMovement = movement;
//                        }
                    }
                }
                if (!workingPiece.empty()) {
                    utility::insertBlock(grid, workingPiece);
                    score += utility::getScore(grid, workingPiece);
                }
//                if (!flag && !grid.exceed()) {
//                    std::cerr << "Invalid movement2;" << temMovement << ", ";
//                }
            }
            totalMovement += numberOfMoves;
            totalScore += score;
        }
//        std::cerr << "totalMovement:" << totalMovement << '\n';
        candidate.fitness = totalScore;
    }

    int randomInteger(int min, int max) {
        return std::uniform_int_distribution<int>(min, max)(gen);
    }

};