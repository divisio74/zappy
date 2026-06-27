#pragma once
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include "Egg.hpp"
#include "Tile.hpp"
#include "Player.hpp"

class GameState {
    public:
        GameState();
        int getWidth() const;
        int getHeight() const;

        Tile &getTile(int x, int y);
        const Tile &getTile(int x, int y) const;

        std::map<int, Player> &getPlayers();
        const std::map<int, Player> &getPlayers() const;
        Player *findPlayer(int id);

        std::vector<Egg> &getEggs();
        const std::vector<Egg> &getEggs() const;
        Egg *findEgg(int id);

        const std::vector<std::string> &getTeams() const;

        int getTimeUnit() const;
        bool isGameOver() const;
        const std::string &getWinner() const;
        std::vector<std::vector<Tile>>& getMap();

        void addEgg(const Egg &egg);
        void removeEgg(int id);

        void setSize(int width, int height);
        void addTeam(const std::string &name);
        void addPlayer(const Player &player);

        void setTimeUnit(int t);
        void setGameOver(const std::string &winner);

        void lock();
        void unlock();

        void addBroadcast(const std::string &teamName, const std::string &msg);
        const std::vector<std::pair<std::string, std::string>> &getBroadcasts() const;
    
        std::vector<int>    _playerToUpt; // si pas bien que ce soit dans GameState alors faire copie player vector dans renderer et comparé (pas opti)
    private:
        int _width;
        int _height;
        std::vector<std::vector<Tile>> _map;
        std::map<int, Player> _players;
        std::vector<Egg> _eggs;
        std::vector<std::string> _teams;
        int _timeUnit;
        bool _gameOver;
        std::string _winner;
        std::mutex _mutex;
        std::vector<std::pair<std::string, std::string>> _broadcasts;
};
