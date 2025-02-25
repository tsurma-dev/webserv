CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -g3
NAME = webserv
MAKEFLAGS = --no-print-directory
BUILD_DIR = ./build
SRC_DIR = ./src

SRC =	main.cpp \
			ServerCreate.cpp \
			ClientHandling.cpp \
			RequestParser.cpp \
			GetResponse.cpp \
			PostResponse.cpp \
			ResponseBuilder.cpp \
			RequestProcessing.cpp \
			DeleteResponse.cpp \
			ParseHeader.cpp \
			TrieNode.cpp \
			Config.cpp \
			ParsedServer.cpp \
			Location.cpp \
			ParseUtils.cpp
			

#OBJ = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC_DIR)/$(SRC))
OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.cpp=.o)))

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	-mkdir -p $(BUILD_DIR)

all: $(NAME)

clean:
	rm -fr $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all
