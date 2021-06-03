CC = g++ -std=c++11
CFLAFS = -g

TRAVEL_MONITOR_CLIENT_FILES = mainTravelMonitorClient.o travelMonitorClient.o util.o monitorCountryPairList.o monitorList.o bloomFilter.o stringList.o date.o statsList.o
MONITOR_SERVER_FILES = mainMonitorServer.o monitorServer.o util.o stringList.o citizen.o tree.o bloomFilter.o date.o skipList.o

all: clean travelMonitorClient monitorServer

# travelMonitor
travelMonitorClient: $(TRAVEL_MONITOR_CLIENT_FILES)
	$(CC) $(CFLAFS) -o travelMonitorClient $(TRAVEL_MONITOR_CLIENT_FILES)

mainTravelMonitor.o:
	$(CC) $(CFLAFS) -c mainTravelMonitorClient.cpp

travelMonitorClient.o:
	$(CC) $(CFLAFS) -c travelMonitorClient.cpp


# Monitor
monitorServer: $(MONITOR_SERVER_FILES)
	$(CC) $(CFLAFS) -o monitorServer $(MONITOR_SERVER_FILES) -lpthread

mainMonitorServer.o:
	$(CC) $(CFLAFS) -c mainMonitorServer.cpp

monitorServer.o:
	$(CC) $(CFLAFS) -c monitorServer.cpp


# General Files
stringList.o:
	$(CC) $(CFLAFS) -c DataStructures/stringList/stringList.cpp

monitorCountryPairList.o:
	$(CC) $(CFLAFS) -c DataStructures/monitorCountryPairList/monitorCountryPairList.cpp

monitorList.o:
	$(CC) $(CFLAFS) -c DataStructures/monitorList/monitorList.cpp

skipList.o:
	$(CC) $(CFLAFS) -c DataStructures/skipList/skipList.cpp

statsList.o:
	$(CC) $(CFLAFS) -c DataStructures/statsList/statsList.cpp

bloomFilter.o:
	$(CC) $(CFLAFS) -c DataStructures/bloomFilter/bloomFilter.cpp

tree.o:
	$(CC) $(CFLAFS) -c DataStructures/binaryAvlTree/tree.cpp

citizen.o:
	$(CC) $(CFLAFS) -c citizenRecords/citizen.cpp

date.o:
	$(CC) $(CFLAFS) -c DataStructures/date/date.cpp

util.o:
	$(CC) $(CFLAFS) -c util.cpp

.PHONY: clean
clean:
	rm -f travelMonitorClient monitorServer $(TRAVEL_MONITOR_CLIENT_FILES) $(MONITOR_SERVER_FILES) logfiles/*