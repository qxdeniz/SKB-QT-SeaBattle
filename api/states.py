from pydantic import BaseModel
from typing import Dict, List


class GameResponse(BaseModel):
    game_id: str


class GameState(BaseModel):
    player1: Dict[str, List]
    player2: Dict[str, List]

class Attack(BaseModel):
    row: int
    col: int
    game_id: str
    player: str

class Ships(BaseModel):
    ships: List[List[int]]