#include "drivers/usb/xhci.h"
#include "kernel/kernel.h"
#include <stdint.h>

#define TRB_NORMAL 1
#define TRB_SETUP 2
#define TRB_DATA 3
#define TRB_STATUS 4
#define TRB_LINK 6
#define TRB_ENABLE_SLOT 9
#define TRB_ADDRESS_DEVICE 11
#define TRB_CONFIGURE_EP 12
#define TRB_EVALUATE_CTX 13
#define TRB_TRANSFER_EVENT 32
#define TRB_CMD_COMPLETE 33
#define TRB_PORT_STATUS 34

typedef struct {
    uint64_t parameter;
    uint32_t status;
    uint32_t control;
} __attribute__((packed)) TRB;

#define RING_SIZE 256
typedef struct {
    TRB trbs[RING_SIZE];
    uint32_t enqueue;
    uint32_t dequeue;
    uint8_t cycle;
} TRBRing;

static volatile XHCICap* cap;
static volatile XHCIOp* op;
static volatile uint32_t* db;
static volatile uint32_t* rt;

static TRBRing commandRing;
static TRBRing eventRing;

static uint64_t dcbaa[256] __attribute__((aligned(64)));

static uint8_t deviceContext[2048] __attribute__((aligned(64)));
static uint8_t inputContext[2176] __attribute__((aligned(64)));

