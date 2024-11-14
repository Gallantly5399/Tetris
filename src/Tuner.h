//
// Created by amemiya7 on 2024/6/24.
//

#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include "Generator.h"

#include "Ai.h"

struct Candidate {
    double heightWeight;
    double linesWeight;
    double holesWeight;
    double bumpinessWeight;
    int fitness;
};

class Tuner {
public:
    void output(const Candidate& candidate) {
        std::cout << "heightWeight:" << candidate.heightWeight << ",linesWeight:" << candidate.linesWeight << ",holesWeight:" << candidate.holesWeight << ",bumpinessWeight:" << candidate.bumpinessWeight << ",fitness:" << candidate.fitness << '\n';
    }
    Tuner() {
        std::random_device rd;
        gen = std::mt19937(rd());
    }

    static void normalize(Candidate &candidate) {
        double norm = std::sqrt(
                candidate.heightWeight * candidate.heightWeight + candidate.linesWeight * candidate.linesWeight +
                candidate.holesWeight * candidate.holesWeight + candidate.bumpinessWeight * candidate.bumpinessWeight);
        //generate a number between 0 and 1


        candidate.heightWeight /= norm;
        candidate.linesWeight /= norm;
        candidate.holesWeight /= norm;
        candidate.bumpinessWeight /= norm;
    }

    Candidate generateRandomCandidate() {
        Candidate candidate = {
                .heightWeight = randomDouble(0, 1),
                .linesWeight = randomDouble(0, 1),
                .holesWeight = randomDouble(0, 1),
                .bumpinessWeight = randomDouble(0, 1)
        };
        normalize(candidate);
        return candidate;
    }

    static void computeFitnesses(std::vector<Candidate> &candidates, int numberOfGames, int maxNumberOfMoves) {
        for (int i = 0; i < candidates.size(); i++) {
            Candidate& candidate = candidates[i];
            AI ai = AI(candidate.heightWeight, candidate.linesWeight, candidate.holesWeight,
                            candidate.bumpinessWeight);
            int totalScore = 0;
            for (int j = 0; j < numberOfGames; j++) {
                Grid grid(10, 22);
                Generator rpg{};
                std::vector<Block> workingPieces = {rpg.nextBlock(), rpg.nextBlock()};
                Block workingPiece = workingPieces[0];

                int score = 0;
                int numberOfMoves = 0;
                while ((numberOfMoves++) < maxNumberOfMoves && !grid.exceed()) {
                    //TODO::ai thread
//                    workingPiece = ai->best(grid, workingPieces).first;
                    while (workingPiece.moveDown(grid));
                    insertBlock(grid, workingPiece);
                    score += grid.clearLines();
                    for (int k = 0; k < workingPieces.size() - 1; k++) {
                        workingPieces[k] = workingPieces[k + 1];
                    }
                    workingPieces[workingPieces.size() - 1] = rpg.nextBlock();
                }
                totalScore += score;
            }
            candidate.fitness = totalScore;
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

    static Candidate crossOver(const Candidate& candidate1, const Candidate& candidate2) {
        double totalFitness = candidate1.fitness + candidate2.fitness;
        double coefficient1 = 0, coefficient2 = 0;
        if (totalFitness == 0) {
            coefficient1 = coefficient2 = 0.5;
        } else {
          coefficient1 = 1.0 * candidate1.fitness / totalFitness;
          coefficient2 = 1.0 * candidate2.fitness / totalFitness;
        }
        Candidate candidate = {
                .heightWeight = coefficient1 * candidate1.heightWeight +
                                coefficient2 * candidate2.heightWeight,
                .linesWeight = coefficient1 * candidate1.linesWeight +
                        coefficient2 * candidate2.linesWeight,
                .holesWeight = coefficient1 * candidate1.holesWeight +
                        coefficient2 * candidate2.holesWeight,
                .bumpinessWeight = coefficient1 * candidate1.bumpinessWeight +
                                coefficient2 * candidate2.bumpinessWeight
        };
        normalize(candidate);
        return candidate;
    }

    void mutate(Candidate &candidate) {
        double quantity = randomDouble(-0.2, 0.2);
        switch (randomInteger(0, 4)) {
            case 0:
                candidate.heightWeight += quantity;
                break;
            case 1:
                candidate.linesWeight += quantity;
                break;
            case 2:
                candidate.holesWeight += quantity;
                break;
            case 3:
                candidate.bumpinessWeight += quantity;
                break;
        }
    }

    static void deleteNLastReplacement(std::vector<Candidate> &candidates, const std::vector<Candidate> &newCandidates) {
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

        computeFitnesses(candidates, 5, 200);
        std::sort(candidates.begin(), candidates.end(), [&](Candidate a, Candidate b) {
            return a.fitness > b.fitness;
        });

        int count = 0;
        while (true) {
            std::vector<Candidate> newCandidates;
            for (int i = 0; i < 30; i++) { // 30% of population
                auto pair = tournamentSelectPair(candidates, 10); // 10% of population
//                std::cout << "First: ";
//                output(pair.first);
//                std::cout << "Second: ";
//                output(pair.second);
                Candidate candidate = crossOver(pair.first, pair.second);
//                std::cout << "Crossed: ";
//                output(candidate);
                if (randomInteger(1, 100) <= 5) {
                    mutate(candidate);
//                    output(candidate);
                }
                normalize(candidate);
                newCandidates.push_back(candidate);
            }
            std::cout << "Computing fitnesses of new candidates. (" + std::to_string(count) + ")\n";
            computeFitnesses(newCandidates, 5, 200);
            deleteNLastReplacement(candidates, newCandidates);
            int totalFitness = 0;
            for (int i = 0; i < candidates.size(); i++) {
                totalFitness += candidates[i].fitness;
            }
            std::cout << "Average fitness = " << 1.0 * totalFitness / candidates.size() << '\n';
            std::cout << "Highest fitness = " << candidates[0].fitness << "(" << count << ")\n";
            std::cout << "Fittest candidate = " << "heightWeight:" << candidates[0].heightWeight << ',' <<
                      "bumpinessWeight:" << candidates[0].bumpinessWeight << ',' << "holesWeight:"
                      << candidates[0].holesWeight << ',' <<
                      "linesWeight:" << candidates[0].linesWeight << '(' << count << ")\n";
            count++;
        }
    };
public:
    std::mt19937 gen;

    int randomInteger(int min, int max) {
        return std::uniform_int_distribution<int>(min, max)(gen);
    }

    double randomDouble(double min, double max) {
        return std::uniform_real_distribution<double>(min, max)(gen);
    }
};