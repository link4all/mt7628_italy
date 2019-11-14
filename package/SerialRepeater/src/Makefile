BUILD_TARGET := SerialRepeater

INCLUDES := -I./ -I./Utils -I./Terminals -I./Required/glib-2.0 -I./Required/glib-2.0/glib
CXXFLAGS += -std=c++11 $(INCLUDES)
LIBS     := -lstdc++ -lcurl -lglib-2.0 -levent -lpthread -lssl -lcrypto -lcares -luci -lubox -lmosquitto -lsqlite3 -lmodbus
XLDFLAGS += $(LDFLAGS) $(LIBS)

OBJS	 := 	main.o \
                uart.o \
                sysconf.o \
                rtu.o \
                models.o \
                wsapi.o \
                service.o \
                baseinfo.o \
                persistent.o \
                profiles.o \
                collect.o \
                mqtt.o \
                status.o \
                tcpclient.o \
                notification_service_routines.o \
                std_modbus_device.o \
                Terminals/FC810.o \
                Terminals/FC890.o \
                Terminals/HM01.o \
                Terminals/FC109R.o \
                Terminals/H3U.o \
                Utils/evloop.o \
                Utils/http.o \
                Utils/log.o \
                Utils/cpuset.o \
                Utils/csvwriter.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: clean $(OBJS)
	$(CXX) $(XLDFLAGS) -o $(BUILD_TARGET) $(OBJS)

clean:
	rm -rf {Utils}/*.o *.o $(BUILD_TARGET)
