CXX ?= g++
APP_NAME = camara
APP_OBJ_FILES = camara.o

LIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs

all: $(APP_NAME)

$(APP_NAME): $(APP_OBJ_FILES)
	$(CXX) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CXX) -c $< -o $@

clean:
	rm -f *.o $(APP_NAME)
