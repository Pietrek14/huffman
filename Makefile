SRC_FILES = main.cpp huffman/huffman.cpp huffman/tree/tree.cpp huffman/message/message.cpp huffman/buffer/buffer.cpp
TARGET_FILE = huffman.exe

build:
	g++ $(SRC_FILES) -o $(TARGET_FILE)

run:
	./$(TARGET_FILE)