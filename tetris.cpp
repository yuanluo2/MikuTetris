#include <algorithm>
#include <initializer_list>
#include <string>
#include <array>
#include <vector>
#include <random>
#include <exception>
#include <stdexcept>
#include <cstdint>
#include <SFML/Graphics.hpp>

constexpr int32_t GAME_MAP_WIDTH = 16;
constexpr int32_t GAME_MAP_HEIGHT = 28;
constexpr int32_t GAME_MAP_EXTRA_HEIGHT = 4;
constexpr int32_t BLOCK_ROW_BEGIN = 2;
constexpr int32_t BLOCK_RENDER_SIDE_LENGTH = 20;

/*
* Classic tetris game has 7 kinds of block shapes: I O T S Z J L
* and I use empty here to represent no block here.
*/
enum class BlockType {
    I, O, T, S, Z, J, L, No_Block_Here
};

/*
* The coordinate system. x increases from left to right, y increases from top to bottom.
* so, yes, y-axis is opposite to the classical Cartesian coordinate system.
*/
struct Coordinate {
    int32_t x;
    int32_t y;

    constexpr Coordinate() : x{0}, y{0} {}
    constexpr Coordinate(int32_t _x, int32_t _y) : x{_x}, y{_y} {}
};

using BlockShape = std::array<Coordinate, 4>;
using BlockShapeTable = std::array<BlockShape, 4>;

constexpr BlockShape make_block_shape(Coordinate c1, Coordinate c2, Coordinate c3, Coordinate c4) {
    BlockShape bs;
    bs[0] = c1;
    bs[1] = c2;
    bs[2] = c3;
    bs[3] = c4;

    return bs;
}

/*
* 4 rotations with each block.
* for example, let's take a look on the first line of T block, it looks like:
* 
*      0           O           O             x axis 
*   (-1, 0)     (0, 0)      (1, 0)
* 
*                  O
*               (0, 1)
* 
* 
*              y  axis
*/
constexpr BlockShapeTable BLOCK_SHAPE_TABLE_I = {
	make_block_shape({ 0, 0 }, { -1, 0 }, {  1,  0 }, {  2,  0 }),
	make_block_shape({ 0, 0 }, { 0, -1 }, {  0,  1 }, {  0,  2 }),
	make_block_shape({ 0, 0 }, { 1,  0 }, { -1,  0 }, { -2,  0 }),
	make_block_shape({ 0, 0 }, { 0,  1 }, {  0, -1 }, {  0, -2 }),
};

constexpr BlockShapeTable BLOCK_SHAPE_TABLE_O = {
	make_block_shape({ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }),
	make_block_shape({ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }),
	make_block_shape({ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }),
	make_block_shape({ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }),
};

constexpr BlockShapeTable BLOCK_SHAPE_TABLE_T = {
	make_block_shape({ 0, 0 }, { -1,  0 }, {  1,  0 }, {  0,  1 }),
	make_block_shape({ 0, 0 }, {  0, -1 }, {  0,  1 }, { -1,  0 }),
	make_block_shape({ 0, 0 }, {  1,  0 }, { -1,  0 }, {  0, -1 }),
	make_block_shape({ 0, 0 }, {  0,  1 }, {  0, -1 }, {  1,  0 }),
};

constexpr BlockShapeTable BLOCK_SHAPE_TABLE_S = {
	make_block_shape({ -1, -1 }, { -1,  0 }, { 0, 0 }, {  0,  1 }),
	make_block_shape({  1, -1 }, {  0, -1 }, { 0, 0 }, { -1,  0 }),
	make_block_shape({  1,  1 }, {  1,  0 }, { 0, 0 }, {  0, -1 }),
	make_block_shape({ -1,  1 }, {  0,  1 }, { 0, 0 }, {  1,  0 }),
};

constexpr BlockShapeTable BLOCK_SHAPE_TABLE_Z = {
	make_block_shape({  0, -1 }, { 0, 0 }, { -1,  0 }, { -1,  1 }),
	make_block_shape({  1,  0 }, { 0, 0 }, {  0, -1 }, { -1, -1 }),
	make_block_shape({  0,  1 }, { 0, 0 }, {  1,  0 }, {  1, -1 }),
	make_block_shape({ -1,  0 }, { 0, 0 }, {  0,  1 }, {  1,  1 }),
};

constexpr BlockShapeTable BLOCK_SHAPE_TABLE_J = {
	make_block_shape({ 0, 0 }, {  0, -1 }, {  0, -2 }, { -1,  0 }),
	make_block_shape({ 0, 0 }, {  1,  0 }, {  2,  0 }, {  0, -1 }),
	make_block_shape({ 0, 0 }, {  0,  1 }, {  0,  2 }, {  1,  0 }),
	make_block_shape({ 0, 0 }, { -1,  0 }, { -2,  0 }, {  0,  1 }),
};

constexpr BlockShapeTable BLOCK_SHAPE_TABLE_L = {
	make_block_shape({ 0, 0 }, {  0, -1 }, {  0, -2 }, {  1,  0 }),
	make_block_shape({ 0, 0 }, {  1,  0 }, {  2,  0 }, {  0,  1 }),
	make_block_shape({ 0, 0 }, {  0,  1 }, {  0,  2 }, { -1,  0 }),
	make_block_shape({ 0, 0 }, { -1,  0 }, { -2,  0 }, {  0, -1 }),
};

class BlockTypeUnknownException : public std::exception {
public:
    const char* what() const noexcept override {
        return "block type unknown";
    }
};

BlockShape const& get_block_shape(BlockType type, uint32_t rotateTimes) {
    switch(type) {
        case BlockType::I: return BLOCK_SHAPE_TABLE_I[rotateTimes];
        case BlockType::O: return BLOCK_SHAPE_TABLE_O[rotateTimes];
        case BlockType::T: return BLOCK_SHAPE_TABLE_T[rotateTimes];
        case BlockType::S: return BLOCK_SHAPE_TABLE_S[rotateTimes];
        case BlockType::Z: return BLOCK_SHAPE_TABLE_Z[rotateTimes];
        case BlockType::J: return BLOCK_SHAPE_TABLE_J[rotateTimes];
        case BlockType::L: return BLOCK_SHAPE_TABLE_L[rotateTimes];
        default: throw BlockTypeUnknownException{};
    }
}

sf::Color get_block_color(BlockType type) {
    switch(type) {
        case BlockType::I: return sf::Color{  57, 197, 187 };
        case BlockType::O: return sf::Color{ 255, 165,   0 };
        case BlockType::T: return sf::Color{ 255, 255,   0 };
        case BlockType::S: return sf::Color{   0, 128,   0 };
        case BlockType::Z: return sf::Color{ 255,   0,   0 };
        case BlockType::J: return sf::Color{   0,   0, 255 };
        case BlockType::L: return sf::Color{ 128,   0, 128 };
        default: throw BlockTypeUnknownException{};
    }
}

struct Block {
    BlockType type;
    uint32_t rotationTimes;
    int32_t row;
    int32_t col;

    BlockShape const& get_shape() const {
        return get_block_shape(type, rotationTimes);
    }

    sf::Color get_color() const {
        return get_block_color(type);
    }
};

/*
* Randomly generate a block.
*/
class BlockGenerator {
	std::mt19937 mt{ std::random_device{}() };
	std::uniform_int_distribution<unsigned int> randomType{ 0, 6 };
	std::uniform_int_distribution<unsigned int> randomRotation{ 0, 3 };
public:
	Block operator()() {
		return Block {
			.type = static_cast<BlockType>(randomType(mt)), 
			.rotationTimes = static_cast<uint32_t>(randomRotation(mt)),
			.row = BLOCK_ROW_BEGIN,
			.col = GAME_MAP_WIDTH / 2
		};
	}
};

class Tetris {
	std::array<std::array<BlockType, GAME_MAP_WIDTH>, GAME_MAP_HEIGHT + GAME_MAP_EXTRA_HEIGHT> gameMap;
	BlockGenerator gen;
	Block currentBlock;
	bool gameOver = false;

	template<typename BorderCrossFunc>
	bool check_collision_base(BorderCrossFunc borderCross) {
		BlockShape const& shape = currentBlock.get_shape();

		for (const Coordinate& coord : shape) {
			int32_t row = currentBlock.row + coord.y;
			int32_t col = currentBlock.col + coord.x;

			if(borderCross(row, col) || gameMap[row][col] != BlockType::No_Block_Here) {
				return true;
			}
		}

		return false;
	}

	bool check_left_collision() {
		return check_collision_base([](int32_t row, int32_t col) {
			return col < 0;
		});
	}

	bool check_right_collision() {
		return check_collision_base([](int32_t row, int32_t col) {
			return col >= GAME_MAP_WIDTH;
		});
	}

	bool check_bottom_collision() {
		return check_collision_base([](int32_t row, int32_t col) {
			return row >= GAME_MAP_HEIGHT + GAME_MAP_EXTRA_HEIGHT;
		});
	}

	bool check_left_right_bottom_collision() {
		return check_collision_base([](int32_t row, int32_t col) {
			return row >= GAME_MAP_HEIGHT + GAME_MAP_EXTRA_HEIGHT || col < 0 || col >= GAME_MAP_WIDTH;
		});
	}

	bool row_is_empty(int32_t rowIndex) {
		const auto& row = gameMap[rowIndex];
		
		return std::all_of(row.cbegin(), row.cend(), [](BlockType type) { 
			return type == BlockType::No_Block_Here; 
		});
	}

	bool row_is_full(int32_t rowIndex) {
		const auto& row = gameMap[rowIndex];
		
		return std::none_of(row.cbegin(), row.cend(), [](BlockType type) { 
			return type == BlockType::No_Block_Here; 
		});
	}

	/* 
	* To accelerate the speed of line elimination and rendering, it's better to find the bottom empty line.
	* This function will search from the middle row, if middle row is all empty, then search down, 
	* else search upper lines.
	*/
	int32_t find_the_bottom_empty_line() {
		// search starts from the middle row.
		int32_t theBottomEmptyLine = (GAME_MAP_HEIGHT + GAME_MAP_EXTRA_HEIGHT) / 2;    

		if (row_is_empty(theBottomEmptyLine)) {
			for (int32_t r = theBottomEmptyLine + 1; r < GAME_MAP_HEIGHT + GAME_MAP_EXTRA_HEIGHT; ++r) {
				if (row_is_empty(r)) {
					theBottomEmptyLine = r;
				}
			}
		}
		else {
			for (int32_t r = theBottomEmptyLine - 1; r >= GAME_MAP_EXTRA_HEIGHT; --r) {
				if (row_is_empty(r)) {
					return r;
				}
			}
		}

		return theBottomEmptyLine;
	}

	void try_eliminate_lines() {
		int32_t theBottomEmptyLine = find_the_bottom_empty_line();

		for (int32_t r = GAME_MAP_HEIGHT + GAME_MAP_EXTRA_HEIGHT - 1; r > theBottomEmptyLine;) {
			// after elimination, the current row is new, we have to review it.
			if (row_is_full(r)) {
				for (int32_t rr = r - 1; rr >= theBottomEmptyLine; --rr) {
					gameMap[rr + 1] = gameMap[rr];
				}
			}
			else {
				// only this row is not full, then --r
				--r;
			}
		}
	}

	void draw_one_rect(sf::RenderTarget& rt, int32_t row, int32_t col, const sf::Color color) {
		sf::RectangleShape rect;

		rect.setPosition(sf::Vector2f{ 
			static_cast<float>(col * BLOCK_RENDER_SIDE_LENGTH), 
			static_cast<float>((row - GAME_MAP_EXTRA_HEIGHT) * BLOCK_RENDER_SIDE_LENGTH) 
		});
        
		rect.setSize(sf::Vector2f{ static_cast<float>(BLOCK_RENDER_SIDE_LENGTH), static_cast<float>(BLOCK_RENDER_SIDE_LENGTH) });
		rect.setFillColor(color);
		rect.setOutlineThickness(1.0f);
		rect.setOutlineColor(sf::Color::White);
		rt.draw(rect);
	}
public:
	Tetris()
	{
		for (auto& row : gameMap) {
			row.fill(BlockType::No_Block_Here);
		}
		
		currentBlock = gen();
	}

	void move_left() {
		currentBlock.col -= 1;

		if (check_left_collision()) {
			currentBlock.col += 1;
		}
	}

	void move_right() {
		currentBlock.col += 1;

		if (check_right_collision()) {
			currentBlock.col -= 1;
		}
	}

	void move_down() {
		currentBlock.row += 1;

		if (check_bottom_collision()) {
			currentBlock.row -= 1;

			// 1. fill the current block into blocks array.
			BlockShape const& shape = currentBlock.get_shape();

			for (const Coordinate& coord : shape) {
				int32_t row = currentBlock.row + coord.y;
				int32_t col = currentBlock.col + coord.x;

				gameMap[row][col] = currentBlock.type;
			}

			// 2. try to erase lines.
			try_eliminate_lines();

			// 3. game over ?
			if (!row_is_empty(GAME_MAP_EXTRA_HEIGHT)) {
				gameOver = true;
			}

			// 4. change current block to a new one.
			currentBlock = gen();
		}
	}

	void rotate() {
		currentBlock.rotationTimes = (currentBlock.rotationTimes + 1) % 4;

		if (check_left_right_bottom_collision()) {
			currentBlock.rotationTimes = (currentBlock.rotationTimes + 3) % 4;
		}
	}

	void render(sf::RenderTarget& rt) {
		// 1. rendering all blocks in the blocks array.
		for (int32_t row = GAME_MAP_EXTRA_HEIGHT; row < GAME_MAP_HEIGHT + GAME_MAP_EXTRA_HEIGHT; ++row) {
			for (int32_t col = 0; col < GAME_MAP_WIDTH; ++col) {
				if (gameMap[row][col] != BlockType::No_Block_Here) {
					draw_one_rect(rt, row, col, get_block_color(gameMap[row][col]));
				}
			}
		}

		// 2. rendering current block separately.
		BlockShape const& shape = currentBlock.get_shape();

		for (const Coordinate& coord : shape) {
			int32_t row = currentBlock.row + coord.y;
			int32_t col = currentBlock.col + coord.x;

			draw_one_rect(rt, row, col, currentBlock.get_color());
		}
	}

	bool is_game_over() const noexcept {
		return gameOver;
	}
};

int main() {
    Tetris tetris;

	sf::RenderWindow window(sf::VideoMode(GAME_MAP_WIDTH * BLOCK_RENDER_SIDE_LENGTH, GAME_MAP_HEIGHT * BLOCK_RENDER_SIDE_LENGTH), "Tetris");
	sf::Clock clock;
	sf::Time accumulator = sf::Time::Zero;
	sf::Time frameRate = sf::seconds(1.f / 30.f);
	uint32_t counter = 0;

	while (window.isOpen()) {
		while (accumulator > frameRate) {
			accumulator -= frameRate;

			// check events.
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window.close();
				}
				else if (event.type == sf::Event::KeyPressed) {
					// blocks can move continuously.
					if (event.key.code == sf::Keyboard::Up) {
						tetris.rotate();
					}
					else if (event.key.code == sf::Keyboard::Down) {
						tetris.move_down();
					}
					else if (event.key.code == sf::Keyboard::Left) {
						tetris.move_left();
					}
					else if (event.key.code == sf::Keyboard::Right) {
						tetris.move_right();
					}
				}
			}

			++counter;
			if (counter == 6) {
				// even we don't press keys, block should fall down automatically.
				tetris.move_down();
				counter = 0;
			}
		}

		// rendering.
		window.clear();
		tetris.render(window);
		window.display();

		if (tetris.is_game_over()) {
			window.close();
		}

		accumulator += clock.restart();
	}

    return 0;
}
