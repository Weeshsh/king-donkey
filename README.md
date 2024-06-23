# Platformer Game Project

This project is a simple platformer game developed using a custom version of the SDL2 library, provided by the University.

## Features

- **Platformer Mechanics**: The player can move left, right, and jump.
- **Ladders**: The player can climb ladders to navigate different levels.
- **Collision Detection**: Proper collision handling with platforms and ladders.
- **Information Display**: Displays game information and controls on the screen.
- **Custom SDL2 Library**: The project uses a custom version of the SDL2 library tailored for educational purposes.

## Controls

- **Arrow Keys**: Move the player left, right, up, and down.
- **Space**: Jump.
- **Esc**: Exit the game.
- **N**: Restart the game.

## Getting Started

### Prerequisites

To build and run the project, you will need:

- A C++ compiler (like `gcc` or `clang`)
- The custom SDL2 library provided by the University

### Building the Project

1. **Clone the repository**:
    ```sh
    git clone https://github.com/Weeshsh/king-donkey
    cd king-donkey
    ```

2. **Set up SDL2 Library**:
    Make sure to include the custom SDL2 library in the correct directory as specified in the code (`./SDL2-2.0.10/include/`).

3. **Compile the code**:
    ```sh
    g++ -o platformer main.cpp -I./SDL2-2.0.10/include -L./SDL2-2.0.10/lib -lSDL2
    ```

### Running the Game

After compiling, you can run the game using:
```sh
./platformer
```

## File Structure

- **main.cpp**: The main game logic and loop.
- **img/**: Directory containing image assets for the game (platform, ladder, character, and charset).
- **SDL2-2.0.10/**: Directory containing the custom SDL2 library.

## Notes

This project is built based on a custom SDL2 library created by the University for educational purposes. Ensure you have this library correctly set up in your project directory.

## Author

Mikolaj Wiszniewski, Student ID: 197925

---

Feel free to update this `README.md` file with any additional information or adjustments based on your specific project setup or requirements.
