# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I./src

# Имя исполняемого файла
TARGET = tcp_app

# Исходные файлы
SRC_DIR = src
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/server_tcp.cpp $(SRC_DIR)/client_tcp.cpp

# Объектные файлы
OBJS = $(SRCS:.cpp=.o)

# Правила сборки
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Правило для компиляции .cpp файлов в .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJS) $(TARGET)

# Фаза линковки
.PHONY: all clean