import pygame
import random
import math
from enum import Enum

# Initialize PyGame
pygame.init()

# Constants
SCREEN_WIDTH = 1440
SCREEN_HEIGHT = 804
NUM_AGENTS_PER_TYPE = 200
FPS = 60

# Create display
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("Rock Paper Scissors Simulation")
clock = pygame.time.Clock()

class Sign(Enum):
    ROCK = 0
    PAPER = 1
    SCISSORS = 2

class Agent:
    def __init__(self, sign, pos_x, pos_y, vel_x, vel_y):
        self.sign = sign
        self.pos_x = pos_x
        self.pos_y = pos_y
        self.vel_x = vel_x
        self.vel_y = vel_y

    def update(self):
        self.pos_x += self.vel_x
        self.pos_y += self.vel_y
        
        # Boundary collision
        if self.pos_x <= 10 or self.pos_x >= SCREEN_WIDTH - 30:
            self.vel_x *= -1
        if self.pos_y <= 10 or self.pos_y >= SCREEN_HEIGHT - 30:
            self.vel_y *= -1

def create_agents():
    agents = []
    
    # Starting positions (like original)
    rock_center = (1000, 600)
    paper_center = (100, 600)
    scissors_center = (600, 100)
    
    centers = [rock_center, paper_center, scissors_center]
    signs = [Sign.ROCK, Sign.PAPER, Sign.SCISSORS]
    
    for i, (sign, center) in enumerate(zip(signs, centers)):
        for _ in range(NUM_AGENTS_PER_TYPE):
            # Random offset from center
            offset_x = random.randint(-50, 50)
            offset_y = random.randint(-50, 50)
            
            # Random velocity
            vel_x = random.uniform(-1.0, 1.0)
            vel_y = random.uniform(-1.0, 1.0)
            
            agent = Agent(
                sign,
                center[0] + offset_x,
                center[1] + offset_y,
                vel_x,
                vel_y
            )
            agents.append(agent)
    
    return agents

def resolve_collision(agent1, agent2):
    if agent1.sign == Sign.ROCK and agent2.sign == Sign.SCISSORS:
        agent2.sign = Sign.ROCK
    elif agent1.sign == Sign.ROCK and agent2.sign == Sign.PAPER:
        agent1.sign = Sign.PAPER
    elif agent1.sign == Sign.PAPER and agent2.sign == Sign.ROCK:
        agent2.sign = Sign.PAPER
    elif agent1.sign == Sign.PAPER and agent2.sign == Sign.SCISSORS:
        agent1.sign = Sign.SCISSORS
    elif agent1.sign == Sign.SCISSORS and agent2.sign == Sign.PAPER:
        agent2.sign = Sign.SCISSORS
    elif agent1.sign == Sign.SCISSORS and agent2.sign == Sign.ROCK:
        agent1.sign = Sign.ROCK

def check_collisions(agents):
    for i in range(len(agents)):
        for j in range(i + 1, len(agents)):
            agent1 = agents[i]
            agent2 = agents[j]
            
            if agent1.sign == agent2.sign:
                continue
                
            dx = agent1.pos_x - agent2.pos_x
            dy = agent1.pos_y - agent2.pos_y
            distance = math.sqrt(dx * dx + dy * dy)
            
            if distance < 20:
                resolve_collision(agent1, agent2)

def load_sprites():
    try:
        rock_sprite = pygame.image.load("r.png")
        paper_sprite = pygame.image.load("p.png")
        scissors_sprite = pygame.image.load("s.png")
        return rock_sprite, paper_sprite, scissors_sprite
    except pygame.error:
        # Fallback to colored rectangles
        rock_sprite = pygame.Surface((20, 20))
        rock_sprite.fill((100, 100, 100))
        
        paper_sprite = pygame.Surface((20, 20))
        paper_sprite.fill((255, 255, 255))
        
        scissors_sprite = pygame.Surface((20, 20))
        scissors_sprite.fill((255, 0, 0))
        
        return rock_sprite, paper_sprite, scissors_sprite

def render_agents(screen, agents, sprites):
    rock_sprite, paper_sprite, scissors_sprite = sprites
    
    for agent in agents:
        if agent.sign == Sign.ROCK:
            screen.blit(rock_sprite, (agent.pos_x, agent.pos_y))
        elif agent.sign == Sign.PAPER:
            screen.blit(paper_sprite, (agent.pos_x, agent.pos_y))
        elif agent.sign == Sign.SCISSORS:
            screen.blit(scissors_sprite, (agent.pos_x, agent.pos_y))

def main():
    agents = create_agents()
    sprites = load_sprites()
    running = True
    
    while running:
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        
        # Update simulation
        for agent in agents:
            agent.update()
        
        check_collisions(agents)
        
        # Render
        screen.fill((0, 0, 0))  # Black background
        render_agents(screen, agents, sprites)
        
        pygame.display.flip()
        clock.tick(FPS)
    
    pygame.quit()

if __name__ == "__main__":
    main()
