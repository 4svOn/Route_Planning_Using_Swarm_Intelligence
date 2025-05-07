#pragma once

#include "util.hpp"

#include <swarm_intelligence/common/CVRP_problem.hpp>

#include <vector>


namespace SI::PSO {
    class TParticle {

    public:
        TParticle(const CVRP::TProblem& problem);
        TParticle(const CVRP::TProblem& problem, const std::vector<TCoordinate>& position);

        TParticle(const TParticle& other) = default;
        TParticle& operator=(const TParticle& other) = default;
        TParticle& operator=(TParticle&& other) = default;

        TDistance TotalDistance() const;
        const CVRP::TRoutes& GetRoutes() const;

        TDistance Evaluate();
        TDistance BestDistance() const;
        const std::vector<TCoordinate>& BestPosition() const;

        void TwoOpt();

        void Update(const TParameters& parameters, const std::vector<TCoordinate>& bestGlobalPosition, bool isDebugPrint = false);
        void DebugPrint(std::ostream& stream);

        static TDistance TwoOpt(const CVRP::TProblem& problem, std::vector<TCoordinate>& position, TDistance oldBestDistance);

    private:
        void InsertSegment(const std::vector<TCoordinate>& source, size_t start, size_t end);
        void Repair();

    private:
        const CVRP::TProblem& Problem_;
        CVRP::TRoutes Routes_; // Routes for multiple vehicles
        std::vector<TCoordinate> Position_;
        std::vector<TCoordinate> Velocity_;
        std::vector<TCoordinate> BestPosition_;
        TDistance BestDistance_; // Best total distance for this particle
        int64_t IterationsWithoutImprovement_;
    };
};