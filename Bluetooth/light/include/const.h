/** Defines for Self Provisioning **/
static const u8_t net_key[16] = {
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xab,
	0xcd,
	0xef,
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xab,
	0xcd,
	0xef,
};
static const u8_t dev_key[16] = {
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xab,
	0xcd,
	0xef,
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xab,
	0xcd,
	0xef,
};
static const u8_t app_key[16] = {
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xab,
	0xcd,
	0xef,
	0x01,
	0x23,
	0x45,
	0x67,
	0x89,
	0xab,
	0xcd,
	0xef,
};

static const u16_t net_idx;
static const u16_t app_idx;
static const u32_t iv_index;

#if !defined(NODE_ADDR)
#define NODE_ADDR 0x0b0c
#endif

#define GROUP_ADDR 0xc000
#define PUBLISHER_ADDR  0x000f

static u8_t flags;
static u16_t addr = NODE_ADDR;