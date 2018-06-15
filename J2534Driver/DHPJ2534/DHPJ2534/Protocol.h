#pragma once

#include "kepler_defs.h"
#include "Benaphore.h"
#include "PeriodicMsg.h"
#include "PeriodicMessageHandler.h"
#include <queue>

#define MAX_RX_BUFFER_SIZE 256
#define MAX_TX_BUFFER_SIZE 256


class CProtocol : CPeriodicMsgCallback
{
public:
	CProtocol(int ProtocolID);
	~CProtocol(void);

	// PassThru function implementations. Here we handle all the common functionality shared by all protocols, otherwise we defer handling to upper level implementation classes.
	virtual int Connect(unsigned long channelId, unsigned long Flags, unsigned long Baudrate);
	virtual int Disconnect() = 0;
	virtual int ReadMsgs(PASSTHRU_MSG * pMsg, unsigned long * pNumMsgs, unsigned long Timeout);
	virtual int WriteMsgs(PASSTHRU_MSG * pMsg, unsigned long * pNumMsgs, unsigned long Timeout);
	virtual int StartPeriodicMsg(PASSTHRU_MSG * pMsg, unsigned long * pMsgID, unsigned long TimeInterval);
	virtual int StopPeriodicMsg(unsigned long MsgID);
	int StartMsgFilter(unsigned long FilterType, PASSTHRU_MSG * pMaskMsg, PASSTHRU_MSG * pPatternMsg, PASSTHRU_MSG * pFlowControlMsg, unsigned long * pFilterID);
	bool SetFilterSuccess(char * msg, int len);
	virtual int StopMsgFilter(unsigned long FilterID);
	virtual int IOCTL(unsigned long IoctlID, void *pInput, void *pOutput);

	// Higher level function will interpret the message
	virtual bool HandleMsg(PASSTHRU_MSG * pMsg, char * flags) = 0;

	// --- Message writing functions ---

	// higher level function provides the necessary flags for constructing message, then calls DoWriteMsg below
	virtual int WriteMsg(PASSTHRU_MSG * pMsg, unsigned long Timeout) = 0;
	
	bool AddMsgToQueue(TX_QUEUE_MESSAGE *pMsg);

	// receive buffer handlers
	PASSTHRU_MSG * PopMessage();			// releases ownership
	int AddToRXBuffer(PASSTHRU_MSG * pMsg);	// takes ownership
	void ClearRXBuffer();
	void ClearTXBuffer();

	int StopPeriodicMessages();
	virtual int DeleteFilters();	// virtual to let ISO15765 handle deleting also its flow filters.

									// getters & setters
	int ProtocolID();
	void SetLoopback(bool loopback);
	bool IsLoopback();
	int GetDatarate(unsigned long * datarate);
	int GetRXMessageCount();
	void SetRXBufferOverflow(bool status);
	bool IsRXBufferOverflow();
	bool IsListening();
	void SetToListen(bool listen);
	bool IsConnected();

	void SetPinSwitched(bool pinSwitchedMode);
	bool IsPinSwitched();
	int SetJ1962Pins(unsigned long pin1, unsigned long pin2);
	int GetJ1962Pins(unsigned long * pin1, unsigned long * pin2);

	bool ParseMsg(char * msg, int len);

	PASSTHRU_MSG * DoParseSardineMsg(char * msg, int len, char * flags);
	PASSTHRU_MSG * DoParseUSBCANMsg(char * msg, int len, char * flags);

	// callback from CPeriodicMsgCallback, when timer has gone off in one of CPeriodicMsg instances
	int SendPeriodicMsg(PASSTHRU_MSG * pMsg, unsigned long Id);

	// callback from CInterceptorCallback. Used when we are sending reactive loopback messages signaled by intercepted message
	// Message is handled as a normal message received 
	int SendInterceptorMessage(PASSTHRU_MSG * pMsg);	// ownership is taken

	


protected:
	virtual int GetIOCTLParam(SCONFIG * pConfig);
	virtual int SetIOCTLParam(SCONFIG * pConfig);
	virtual int DoReadMsgs(PASSTHRU_MSG * pMsgs, unsigned int count, bool isOverflow);

private:

	// Create copy of the outgoing message and put it in the receiving buffer
	int AddLoopbackMsg(PASSTHRU_MSG * pMsg); // doesn't take ownership

	int DoAddToRXBuffer(PASSTHRU_MSG * pMsg);	// takes ownership
	

	CPeriodicMessageHandler * periodicMsgHandler;

	int protocolID;
	Benaphore rx_lock;
	Benaphore tx_lock;
	PASSTHRU_MSG * rxbuffer[MAX_RX_BUFFER_SIZE];
	//PASSTHRU_MSG * txbuffer[MAX_RX_BUFFER_SIZE];
	std::queue<PASSTHRU_MSG*> txBuffer;

	int lowIndex, hiIndex;
	int rxBufferSize;
	bool rxBufferOverflow;
	bool listening;
	bool loopback;
	unsigned long datarate;
	bool pinSwitched;

	unsigned long J1962Pin1;
	unsigned long J1962Pin2;

	unsigned long rollingPeriodicMsgId;

	// these are used if functionality not implemented by higher level protocol, but if we want silent failing
	int _dummy_filter_id;

	unsigned long channelId;

	int FilterSetSuccessfull; //0 - waiting 1 - success 2 - failed
	
};

