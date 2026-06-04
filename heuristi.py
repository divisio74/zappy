import random

def heuristik(state):
    if "food" in state["vision"][0]:
        return "recolt food"
    else :
        return random.choice(["Forward", "Forward", "Left", "Right"])

test_state = {"vision": [["food"], [], []]}
print(heuristik(test_state))
test_state = {"vision": [[], ["food"], []]}
print(heuristik(test_state))
