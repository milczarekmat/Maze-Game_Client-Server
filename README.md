# Maze-Client-Server-Game

## Specifications of the game
Players (1, 2, 3, 4) are locked in a maze and are tasked with collecting appearing treasures in the form of coins (c, t, T). 
The player who collects enough treasures carries them to the encampment ( A ) and leaves them there. A player can carry any number of coins at a time (carried) but can lose them due to a wild beast attack (*) or by colliding with another player.

In the case of a wild beast attack, the player dies (deaths) and the loot he collected remains at the place of death (D). The player respawns at his starting point.
In the case of a collision with another player, the loot of both remains at the point of collision (D), and the players respawn at their starting points.
The loot left behind (D) has a value. In the case of a collision, it is the sum of the carried coins of both players.
The player disposes of his coins at the encampment (A), where they are credited to his account (budget). After passing the treasure, the player continues the search, starting from the encampment.

### Used mechanisms
I used multithreaded programming to create this game. I learned about the term race conditions and what they are all about. In order to prevent their occurrence, I used mutexes. For client-server communication I used the network socket API.


## Game view
![GRA-PROJEKT-SS](https://user-images.githubusercontent.com/97130903/232145653-0bc4494d-e21e-4226-82e9-94475cae45e0.png)
