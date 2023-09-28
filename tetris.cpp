#include <algorithm>
#include <array>
#include <random>
#include <cstdint>
#include <SFML/Graphics.hpp>

using std::array;

/*
* Classic tetris game has 7 kinds of block shapes: I O T S Z J L
* and I use empty here to represent no block here.
* They will be cast as array's index.
*/
enum class Block : int16_t {
	I, O, T, S, Z, J, L, Empty
};

/*
* The coordinate system. x increases from left to right, y increases from top to bottom.
* so, yes, y-axis is opposite to the classical Cartesian coordinate system.
*/
struct Coordinate {
	int16_t x = 0, y = 0;

	constexpr Coordinate() = default;
	constexpr Coordinate(int16_t _x, int16_t _y) : x{ _x }, y{ _y } {};
};

/*
* Some constants assosicated with the game.
* EXTRA_HEIGHT used here is to achieve the effect of blocks slowly emerging from the top, that is, 
* as time increases, the blocks gradually emerge from a partial exposure to a complete exposure.
*/
constexpr int16_t TETRIS_WIDTH = 16;
constexpr int16_t TETRIS_HEIGHT = 28;
constexpr int16_t EXTRA_HEIGHT = 4;
constexpr int16_t BLOCK_SIZE = 20;

using BlockCoord = array<Coordinate, 4>;

/*
* blocks and their coordinates, each block has 4 rotations. 
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
constexpr array<array<BlockCoord, 4>, 7> blockShapeMapping = {
	// I
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ -1, 0 }, Coordinate{ 1, 0 }, Coordinate{ 2, 0 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 0, -1 }, Coordinate{ 0, 1 }, Coordinate{ 0, 2 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ -1, 0 }, Coordinate{ -2, 0 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 0, 1 }, Coordinate{ 0, -1 }, Coordinate{ 0, -2 } } },
	// O
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ 0, 1 }, Coordinate{ 1, 1 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ 0, 1 }, Coordinate{ 1, 1 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ 0, 1 }, Coordinate{ 1, 1 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ 0, 1 }, Coordinate{ 1, 1 } } },
	// T
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ -1, 0 }, Coordinate{ 1, 0 }, Coordinate{ 0, 1 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 0, -1 }, Coordinate{ 0, 1 }, Coordinate{ -1, 0 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ -1, 0 }, Coordinate{ 0, -1 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 0, 1 }, Coordinate{ 0, -1 }, Coordinate{ 1, 0 } } },
	// S
	BlockCoord{ { Coordinate{ -1, -1 }, Coordinate{ -1, 0 }, Coordinate{ 0, 0 }, Coordinate{ 0, 1 } } },
	BlockCoord{ { Coordinate{ 1, -1 }, Coordinate{ 0, -1 }, Coordinate{ 0, 0 }, Coordinate{ -1, 0 } } },
	BlockCoord{ { Coordinate{ 1, 1 }, Coordinate{ 1, 0 }, Coordinate{ 0, 0 }, Coordinate{ 0, -1 } } },
	BlockCoord{ { Coordinate{ -1, 1 }, Coordinate{ 0, 1 }, Coordinate{ 0, 0 }, Coordinate{ 1, 0 } } },
	// Z
	BlockCoord{ { Coordinate{ 0, -1 }, Coordinate{ 0, 0 }, Coordinate{ -1, 0 }, Coordinate{ -1, 1 } } },
	BlockCoord{ { Coordinate{ 1, 0 }, Coordinate{ 0, 0 }, Coordinate{ 0, -1 }, Coordinate{ -1, -1 } } },
	BlockCoord{ { Coordinate{ 0, 1 }, Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ 1, -1 } } },
	BlockCoord{ { Coordinate{ -1, 0 }, Coordinate{ 0, 0 }, Coordinate{ 0, 1 }, Coordinate{ 1, 1 } } },
	// J
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 0, -1 }, Coordinate{ 0, -2 }, Coordinate{ -1, 0 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ 2, 0 }, Coordinate{ 0, -1 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 0, 1 }, Coordinate{ 0, 2 }, Coordinate{ 1, 0 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ -1, 0 }, Coordinate{ -2, 0 }, Coordinate{ 0, 1 } } },
	// L
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 0, -1 }, Coordinate{ 0, -2 }, Coordinate{ 1, 0 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 1, 0 }, Coordinate{ 2, 0 }, Coordinate{ 0, 1 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ 0, 1 }, Coordinate{ 0, 2 }, Coordinate{ -1, 0 } } },
	BlockCoord{ { Coordinate{ 0, 0 }, Coordinate{ -1, 0 }, Coordinate{ -2, 0 }, Coordinate{ 0, -1 } } },
};

/*
* blocks and their colors.
*/
const array<sf::Color, 7> blockColorMapping = {
	// I (miku color #39C5BB)
	sf::Color{  57, 197, 187 },
	// O
	sf::Color{ 255, 165,   0 },
	// T
	sf::Color{ 255, 255,   0 },
	// S
	sf::Color{   0, 128,   0 },
	// Z
	sf::Color{ 255,   0,   0 },
	// J
	sf::Color{   0,   0, 255 },
	// L
	sf::Color{ 128,   0, 128 } 
};

/*
* This class will record a block and its rotation, current row and column.
*/
struct BlockInfo {
	Block block;
	int16_t rotation, row, col;

	BlockInfo() = default;

	BlockInfo(Block _block, int16_t _rotation, int16_t _row, int16_t _col) :
		block{ _block }, rotation{ _rotation }, row{ _row }, col{ _col }
	{}

	const BlockCoord& getCoord() const noexcept {
		return blockShapeMapping[static_cast<int16_t>(block)][rotation];
	}

	const sf::Color& getColor() const noexcept {
		return blockColorMapping[static_cast<int16_t>(block)];
	}
};

/*
* Randomly generate a block.
*/
class BlockGen {
	std::mt19937 mt{ std::random_device{}() };
	std::uniform_int_distribution<unsigned int> randomBlock{ 0, 6 };
	std::uniform_int_distribution<unsigned int> randomRotation{ 0, 3 };
public:
	BlockGen() = default;

	BlockInfo operator()() {
		BlockInfo blockInfo{};
		blockInfo.block = static_cast<Block>(randomBlock(mt));
		blockInfo.rotation = static_cast<int16_t>(randomRotation(mt));
		blockInfo.row = 2;    // every new block will begin to fall down at this row, not 0.
		blockInfo.col = TETRIS_WIDTH / 2;

		return blockInfo;
	}
};

class Tetris {
	array<array<Block, TETRIS_WIDTH>, TETRIS_HEIGHT + EXTRA_HEIGHT> blocks;
	BlockInfo currentBlockInfo;
	BlockGen blockGen;
	bool gameOver;

	// only check left side.
	bool checkLeftCollision() {
		const BlockCoord& blockCoord = currentBlockInfo.getCoord();
		for (const auto& coord : blockCoord) {
			int16_t row = currentBlockInfo.row + coord.y;
			int16_t col = currentBlockInfo.col + coord.x;
			if (col < 0 || blocks[row][col] != Block::Empty) {
				return true;
			}
		}

		return false;
	}

	// only check right side.
	bool checkRightCollision() {
		const BlockCoord& blockCoord = currentBlockInfo.getCoord();
		for (const auto& coord : blockCoord) {
			int16_t row = currentBlockInfo.row + coord.y;
			int16_t col = currentBlockInfo.col + coord.x;
			if (col >= TETRIS_WIDTH || blocks[row][col] != Block::Empty) {
				return true;
			}
		}

		return false;
	}

	// only check bottom.
	bool checkBottomCollision() {
		const BlockCoord& blockCoord = currentBlockInfo.getCoord();
		for (const auto& coord : blockCoord) {
			int16_t row = currentBlockInfo.row + coord.y;
			int16_t col = currentBlockInfo.col + coord.x;

			if (row >= TETRIS_HEIGHT + EXTRA_HEIGHT || blocks[row][col] != Block::Empty) {
				return true;
			}
		}

		return false;
	}

	// check left, right, bottom.
	bool checkAllCollision() {
		const BlockCoord& blockCoord = currentBlockInfo.getCoord();
		for (const auto& coord : blockCoord) {
			int16_t row = currentBlockInfo.row + coord.y;
			int16_t col = currentBlockInfo.col + coord.x;

			if (row >= TETRIS_HEIGHT + EXTRA_HEIGHT || col < 0 || col >= TETRIS_WIDTH || blocks[row][col] != Block::Empty) {
				return true;
			}
		}

		return false;
	}

	// the elements on this row is all Block::Empty ?
	bool rowIsEmpty(int16_t rowIndex) {
		const auto& row = blocks[rowIndex];
		return std::all_of(row.cbegin(), row.cend(), [](Block block) { return block == Block::Empty; });
	}

	// the elements on this row is all not Block::Empty ?
	bool rowIsFull(int16_t rowIndex) {
		const auto& row = blocks[rowIndex];
		return std::none_of(row.cbegin(), row.cend(), [](Block block) { return block == Block::Empty; });
	}

	/* 
	* To accelerate the speed of line elimination and rendering, it's better to find the bottom empty line.
	* This function will search from the middle row, if middle row is all empty, then search down, 
	* else search upper lines.
	*/
	int16_t findTheBottomEmptyLine() {
		int16_t theBottomEmptyLine = (TETRIS_HEIGHT + EXTRA_HEIGHT) / 2;    // search starts from the middle row.

		if (rowIsEmpty(theBottomEmptyLine)) {
			for (int16_t r = theBottomEmptyLine + 1; r < TETRIS_HEIGHT + EXTRA_HEIGHT; ++r) {
				if (rowIsEmpty(r)) {
					theBottomEmptyLine = r;
				}
			}
		}
		else {
			for (int16_t r = theBottomEmptyLine - 1; r >= EXTRA_HEIGHT; --r) {
				if (rowIsEmpty(r)) {
					return r;
				}
			}
		}

		return theBottomEmptyLine;
	}

	// line elimination.
	void tryEliminateLines() {
		int16_t theBottomEmptyLine = findTheBottomEmptyLine();

		for (int16_t r = TETRIS_HEIGHT + EXTRA_HEIGHT - 1; r > theBottomEmptyLine;) {
			// after elimination, the current row is new, we have to review it.
			if (rowIsFull(r)) {
				for (int16_t rr = r - 1; rr >= theBottomEmptyLine; --rr) {
					blocks[rr + 1] = blocks[rr];
				}
			}
			else {
				// only this row is not full, then --r
				--r;
			}
		}
	}

	// drawing a rectangle at given position.
	void drawRect(sf::RenderTarget& rt, int16_t row, int16_t col, const sf::Color color) {
		sf::RectangleShape rect;

		rect.setPosition(sf::Vector2f{ static_cast<float>(col * BLOCK_SIZE), static_cast<float>((row - EXTRA_HEIGHT) * BLOCK_SIZE) });
		rect.setSize(sf::Vector2f{ static_cast<float>(BLOCK_SIZE), static_cast<float>(BLOCK_SIZE) });
		rect.setFillColor(color);
		rect.setOutlineThickness(1.0f);
		rect.setOutlineColor(sf::Color::White);
		rt.draw(rect);
	}
public:
	Tetris() : blocks{}, currentBlockInfo{}, blockGen{}, gameOver{ false } {
		for (auto& row : blocks) {
			row.fill(Block::Empty);
		}

		currentBlockInfo = blockGen();
	}

	void moveLeft() {
		currentBlockInfo.col -= 1;

		if (checkLeftCollision()) {
			currentBlockInfo.col += 1;
		}
	}

	void moveRight() {
		currentBlockInfo.col += 1;

		if (checkRightCollision()) {
			currentBlockInfo.col -= 1;
		}
	}

	void moveDown() {
		currentBlockInfo.row += 1;

		if (checkBottomCollision()) {
			currentBlockInfo.row -= 1;

			// 1. fill the current block into blocks array.
			const BlockCoord& blockCoord = currentBlockInfo.getCoord();
			for (const auto& coord : blockCoord) {
				int16_t row = currentBlockInfo.row + coord.y;
				int16_t col = currentBlockInfo.col + coord.x;

				blocks[row][col] = currentBlockInfo.block;
			}

			// 2. try to erase lines.
			tryEliminateLines();

			// 3. game over ?
			if (!rowIsEmpty(EXTRA_HEIGHT)) {
				gameOver = true;
			}

			// 4. change current block to a new one.
			currentBlockInfo = blockGen();
		}
	}

	void rotate() {
		currentBlockInfo.rotation = (currentBlockInfo.rotation + 1) % 4;

		if (checkAllCollision()) {
			currentBlockInfo.rotation = (currentBlockInfo.rotation + 3) % 4;
		}
	}

	void render(sf::RenderTarget& rt) {
		// 1. rendering all blocks in the blocks array.
		for (int16_t row = EXTRA_HEIGHT; row < TETRIS_HEIGHT + EXTRA_HEIGHT; ++row) {
			for (int16_t col = 0; col < TETRIS_WIDTH; ++col) {
				if (blocks[row][col] != Block::Empty) {
					drawRect(rt, row, col, blockColorMapping[static_cast<int16_t>(blocks[row][col])]);
				}
			}
		}

		// 2. rendering current block separately.
		const BlockCoord& blockCoord = currentBlockInfo.getCoord();
		for (const auto& coord : blockCoord) {
			int16_t row = currentBlockInfo.row + coord.y;
			int16_t col = currentBlockInfo.col + coord.x;
			drawRect(rt, row, col, blockColorMapping[static_cast<int16_t>(currentBlockInfo.block)]);
		}
	}

	bool isGameOver() const noexcept {
		return gameOver;
	}
};

int main() {
	Tetris tetris;

	sf::RenderWindow window(sf::VideoMode(TETRIS_WIDTH * BLOCK_SIZE, TETRIS_HEIGHT * BLOCK_SIZE), "MikuTetris");
	sf::Clock clock;
	sf::Time accumulator = sf::Time::Zero;
	sf::Time frameRate = sf::seconds(1.f / 30.f);
	int16_t counter = 0;

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
						tetris.moveDown();
					}
					else if (event.key.code == sf::Keyboard::Left) {
						tetris.moveLeft();
					}
					else if (event.key.code == sf::Keyboard::Right) {
						tetris.moveRight();
					}
				}
			}

			++counter;
			if (counter == 6) {
				// even we don't press keys, block should fall down automatically.
				tetris.moveDown();
				counter = 0;
			}
		}

		// rendering.
		window.clear();
		tetris.render(window);
		window.display();

		if (tetris.isGameOver()) {
			window.close();
		}

		accumulator += clock.restart();
	}
}
