#pragma once
#include <vector>
#include <string>
#include <mutex>
#include "Egg.hpp"
#include "Tiles.hpp"
#include "Player.hpp"

class GameState {
    public:
        GameState();
        int getWidth() const;
        int getHeight() const;

        Tile &getTile(int x, int y);
        const Tile &getTile(int x, int y) const;

        std::vector<Player> &getPlayers();
        const std::vector<Player> &getPlayers() const;
        Player *findPlayer(uint32_t id);

        std::vector<Egg> &getEggs();
        const std::vector<Egg> &getEggs() const;
        Egg *findEgg(uint32_t id);

        const std::vector<std::string> &getTeams() const;

        int getTimeUnit() const;
        bool isGameOver() const;
        const std::string &getWinner() const;

        void addEgg(const Egg &egg);
        void removeEgg(uint32_t id);

        void setSize(int width, int height);
        void addTeam(const std::string &name);
        void addPlayer(const Player &player);

        void setTimeUnit(int t);
        void setGameOver(const std::string &winner);

        void lock();
        void unlock();

    private:
        int _width;
        int _height;
        std::vector<std::vector<Tile>> _map;
        std::vector<Player> _players;
        std::vector<Egg> _eggs;
        std::vector<std::string> _teams;
        int _timeUnit;
        bool _gameOver;
        std::string _winner;
        std::mutex _mutex;
};
