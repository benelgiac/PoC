#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=clang
CCC=clang++
CXX=clang++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=CLang-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/QAppNG/Imsi.o \
	${OBJECTDIR}/QAppNG/LightWeightSequencerEvo.o \
	${OBJECTDIR}/QAppNG/PeriodicTimer.o \
	${OBJECTDIR}/QAppNG/PipedProcess.o \
	${OBJECTDIR}/QAppNG/QStatusManager.o \
	${OBJECTDIR}/QAppNG/QVirtualClock.o \
	${OBJECTDIR}/QAppNG/TablesHandler.o \
	${OBJECTDIR}/QAppNG/TablesRenderer.o \
	${OBJECTDIR}/QAppNG/ThreadCounter.o \
	${OBJECTDIR}/QAppNG/TicketsCommonMethods.o \
	${OBJECTDIR}/QAppNG/TrivialCircularLockFreeQueueEvo.o \
	${OBJECTDIR}/QAppNG/WorkManager.o \
	${OBJECTDIR}/QAppNG/WorkManagerStatus.o \
	${OBJECTDIR}/QAppNG/nl_clockable_time.o \
	${OBJECTDIR}/QAppNG/nl_osal.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/pugixml/pugixml.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lboost_system -lpthread

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/poc

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/poc: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/poc ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/QAppNG/Imsi.o: QAppNG/Imsi.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/Imsi.o QAppNG/Imsi.cpp

${OBJECTDIR}/QAppNG/LightWeightSequencerEvo.o: QAppNG/LightWeightSequencerEvo.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/LightWeightSequencerEvo.o QAppNG/LightWeightSequencerEvo.cpp

${OBJECTDIR}/QAppNG/PeriodicTimer.o: QAppNG/PeriodicTimer.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/PeriodicTimer.o QAppNG/PeriodicTimer.cpp

${OBJECTDIR}/QAppNG/PipedProcess.o: QAppNG/PipedProcess.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/PipedProcess.o QAppNG/PipedProcess.cpp

${OBJECTDIR}/QAppNG/QStatusManager.o: QAppNG/QStatusManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/QStatusManager.o QAppNG/QStatusManager.cpp

${OBJECTDIR}/QAppNG/QVirtualClock.o: QAppNG/QVirtualClock.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/QVirtualClock.o QAppNG/QVirtualClock.cpp

${OBJECTDIR}/QAppNG/TablesHandler.o: QAppNG/TablesHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/TablesHandler.o QAppNG/TablesHandler.cpp

${OBJECTDIR}/QAppNG/TablesRenderer.o: QAppNG/TablesRenderer.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/TablesRenderer.o QAppNG/TablesRenderer.cpp

${OBJECTDIR}/QAppNG/ThreadCounter.o: QAppNG/ThreadCounter.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/ThreadCounter.o QAppNG/ThreadCounter.cpp

${OBJECTDIR}/QAppNG/TicketsCommonMethods.o: QAppNG/TicketsCommonMethods.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/TicketsCommonMethods.o QAppNG/TicketsCommonMethods.cpp

${OBJECTDIR}/QAppNG/TrivialCircularLockFreeQueueEvo.o: QAppNG/TrivialCircularLockFreeQueueEvo.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/TrivialCircularLockFreeQueueEvo.o QAppNG/TrivialCircularLockFreeQueueEvo.cpp

${OBJECTDIR}/QAppNG/WorkManager.o: QAppNG/WorkManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/WorkManager.o QAppNG/WorkManager.cpp

${OBJECTDIR}/QAppNG/WorkManagerStatus.o: QAppNG/WorkManagerStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/WorkManagerStatus.o QAppNG/WorkManagerStatus.cpp

${OBJECTDIR}/QAppNG/nl_clockable_time.o: QAppNG/nl_clockable_time.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/nl_clockable_time.o QAppNG/nl_clockable_time.cpp

${OBJECTDIR}/QAppNG/nl_osal.o: QAppNG/nl_osal.cpp 
	${MKDIR} -p ${OBJECTDIR}/QAppNG
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QAppNG/nl_osal.o QAppNG/nl_osal.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/pugixml/pugixml.o: pugixml/pugixml.cpp 
	${MKDIR} -p ${OBJECTDIR}/pugixml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./ -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pugixml/pugixml.o pugixml/pugixml.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/poc

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
