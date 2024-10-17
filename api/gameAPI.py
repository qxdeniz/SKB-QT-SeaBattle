from fastapi import FastAPI

import uuid
import random
from states import *


app = FastAPI()
games = {}



@app.post("/api/create_game")
async def create_game(format: str = "pvp"): 
    game_id = str(uuid.uuid4())
    games[game_id] = {
        "player1": {"ships": [], "hits": [], "misses": []},
        "player2": {"ships": [], "hits": [], "misses": []} if format == "pvp" else {"ai": True, "ships": [], "hits": [], "misses": []}
    }
    if format == 'pvc':
        await place_ai_ships(game_id=game_id)
    print(game_id)
    return GameResponse(game_id=game_id)

@app.get("/api/available_games")
async def available_games():
    return {"games": list(games.keys())}


@app.get("/api/start_game")
async def start_game(game_id: str, format: str):
    game = games.get(game_id)
    if not game:
        return {"error": "Game not found"}
    if format == 'pvc':
        game["player2"]["ai"] = True
    return {"message": "Game started"}


async def place_ai_ships(game_id: str):
    ai_ships = [[random.randint(0, 10), random.randint(0, 10)] for _ in range(10)]
    games[game_id]["player2"]["ships"] = ai_ships
    print(ai_ships)




@app.get("/api/get_player_field")
async def get_player_field(game_id: str, player: str):
    game = games.get(game_id)
    if not game:
        return {"error": "Game not found"}
    
    ships = game[player]["ships"]
    return {"ships": ships}

@app.post("/api/place_ships")
async def place_ships(game_id: str, player: str, ships: Ships):
    game = games.get(game_id)
    if not game:
        return {"error": "Game not found"}

    game[player]["ships"] = ships.ships
    return {"message": "Ships placed successfully"}


def check_victory(player_ships: List[List[int]], hits: List[List[int]]) -> bool:
    return all(ship in hits for ship in player_ships)

def ai_move(game_id: str):
    game = games[game_id]
    player1_hits = game["player1"]["hits"]
    player1_misses = game["player1"]["misses"]
    
    available_cells = [[r, c] for r in range(10) for c in range(10)
                       if [r, c] not in player1_hits + player1_misses]
    if available_cells:
        return random.choice(available_cells)
    return None

@app.post("/api/attack")
async def attack(attack: Attack):
    game = games.get(attack.game_id)
    if not game:
        return {"error": "Game not found"}

    opponent = "player1" if attack.player == "player2" else "player2"
    player = attack.player


    if [attack.row, attack.col] in game[opponent]["ships"]:
        game[opponent]["hits"].append([attack.row, attack.col])
        result = "hit"
    else:
        game[opponent]["misses"].append([attack.row, attack.col])
        result = "miss"

    if check_victory(game[opponent]["ships"], game[opponent]["hits"]):
        return {
            "result": result,
            "victory": True,
            "winner": player,
            "row": attack.row,
            "col": attack.col,
        }

    if game["player2"].get("ai", False) and player == "player1":
        ai_row, ai_col = ai_move(attack.game_id)

        if [ai_row, ai_col] in game["player1"]["ships"]:
            game["player1"]["hits"].append([ai_row, ai_col])
            ai_result = "hit"
        else:
            game["player1"]["misses"].append([ai_row, ai_col])
            ai_result = "miss"

        if check_victory(game["player1"]["ships"], game["player1"]["hits"]):
            return {
                "result": result,
                "ai_result": ai_result,
                "victory": True,
                "winner": "player2",
                "ai_row": ai_row,
                "ai_col": ai_col,
            }

        return {
            "result": result,
            "ai_result": ai_result,
            "victory": False,
            "ai_row": ai_row,
            "ai_col": ai_col,
            "row": attack.row,
            "col": attack.col,
        }

    return {"result": result, "victory": False, "row": attack.row, "col": attack.col}
