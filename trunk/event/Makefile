DIR_SRC = ./src
DIR_OBJ = ./obj
SUBDIRS = ./extor
TARGET  = event_manager
EXEC_DIR = /data05/event/exec 

DIR_LIB = -L/home/newstech/bin -L./extor/zky_cut
DIR_INC = -I./include -I./extor/include -I./extor/zky_cut -I./doc_storage

SRCS = $(wildcard $(DIR_SRC)/*.cpp)
OBJS = $(patsubst %.cpp, $(DIR_OBJ)/%.o, $(notdir $(SRCS)))
CC = g++
#CFLAGS = -g -Wall  
CFLAGS = -g
all:subdir $(TARGET)
	cp $(TARGET) $(EXEC_DIR)
subdir:
	cd $(SUBDIRS) && make

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ ./lib/libextor.a $(DIR_LIB) -lNLPIR -lpthread -luuid
$(DIR_OBJ)/%.o:$(DIR_SRC)/%.cpp
	$(CC) -c $(CFLAGS) -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $< $(DIR_INC)

sinclude $(OBJS:%.o=%.d)
clean:
	cd $(SUBDIRS) && make clean
	-rm $(TARGET) $(OBJS) $(OBJS:%.o=%.d)
