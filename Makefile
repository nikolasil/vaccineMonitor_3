CC = g++ -std=c++11
CFLAFS = -g

TRAVEL_MONITOR_FILES = mainTravelMonitor.o travelMonitor.o util.o monitorCountryPairList.o monitorList.o bloomFilter.o stringList.o date.o statsList.o
MONITOR_FILES = mainMonitor.o monitor.o util.o stringList.o citizen.o tree.o bloomFilter.o date.o skipList.o

all: clean travelMonitor Monitor

# travelMonitor
travelMonitor: $(TRAVEL_MONITOR_FILES)
	$(CC) $(CFLAFS) -o travelMonitor $(TRAVEL_MONITOR_FILES)

mainTravelMonitor.o:
	$(CC) $(CFLAFS) -c mainTravelMonitor.cpp

travelMonitor.o:
	$(CC) $(CFLAFS) -c travelMonitor.cpp


# Monitor
Monitor: $(MONITOR_FILES)
	$(CC) $(CFLAFS) -o Monitor $(MONITOR_FILES)

mainMonitor.o:
	$(CC) $(CFLAFS) -c mainMonitor.cpp

monitor.o:
	$(CC) $(CFLAFS) -c monitor.cpp


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
	rm -f travelMonitor Monitor $(TRAVEL_MONITOR_FILES) $(MONITOR_FILES) pipes/* logfiles/*