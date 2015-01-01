/** @file alloc.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define NUM_SIZES 24

int iflag = 1;

size_t table_size[] = { 4, 16, 32, 64, 128,
						256, 512, 1024, 2048, 4096,
						8192, 16384, 32768, 40960, 49152,
						57344, 65536, 73728, 81920, 90112,
						98304, 106496, 114688, 122880};

typedef struct head_t {
	size_t num;
	struct bloc_t * payload;
} head_t;


typedef struct bloc_t {
	size_t size;
	struct bloc_t * next;
} bloc_t;

head_t ** flist = NULL;

size_t align( size_t raw ) {
	//puts("Aligned!");
	//printf("%lu\n", 3* sizeof(bloc_t) + ((raw + 7) & ~7));
	return ( 30 * sizeof(bloc_t) + ((raw + 7) & ~7));
}

int find_idx(size_t size)
{
	int i;
	for( i = NUM_SIZES; i > 1; i--) {
		if( size > table_size[i-2] ) {
			//printf("%d\n", i-1);
			return i-1;
		}
	}

	return 0;
}

void init() {
	flist = sbrk( NUM_SIZES * sizeof(head_t *) );

	int i;
	for( i = 0; i < NUM_SIZES; i++)
	{
		flist[i] = sbrk( sizeof(head_t) ); 
		flist[i]->num = 0;
		flist[i]->payload = NULL;
	}
	return;
}

void * split(bloc_t * chunk, size_t size) {
	size_t chunk_size = chunk->size;
	void * loc = (void *)(chunk+1);
	
	loc += size;
	bloc_t * spawn_1 = (bloc_t *)(loc);
	spawn_1->size = size;

	loc += sizeof(bloc_t) + size;
	bloc_t * spawn_2 = (bloc_t *)(loc);
	spawn_2->size = size;

	int idx = find_idx(size);

	spawn_1->next = spawn_2;
	spawn_2->next = flist[idx]->payload;
	flist[idx]->payload = spawn_1;

	flist[idx]->num += 2;

	return (void *)(chunk+1);
}

void * bs(bloc_t * chunk) {
	if(chunk->size < 168956970 )
		return (void *)(chunk+1);
	size_t chunk_size = chunk->size + sizeof(bloc_t);
	size_t bloc_size = chunk_size/3;
	chunk->size = bloc_size;

	void * loc = (void *)(chunk) + bloc_size;

	bloc_t * spawn_1 = (bloc_t *)(loc);
	spawn_1->size = bloc_size - sizeof(bloc_t);

	loc += bloc_size;
	bloc_t * spawn_2 = (bloc_t *)(loc);
	spawn_2->size = bloc_size - sizeof(bloc_t);

	int idx = NUM_SIZES -1;

	spawn_1->next = spawn_2;
	spawn_2->next = flist[idx]->payload;
	flist[idx]->payload = spawn_1;

	flist[idx]->num += 2;

	bloc_t * trav = flist[NUM_SIZES-1]->payload;

	return (void *)(chunk+1);
}

void * hunt(size_t size)
{
	int idx = find_idx(size);
	size_t count = flist[idx]->num;
	bloc_t * prey = NULL;

	if(idx == NUM_SIZES -1 && !count)
	{
		size_t expended = align(size);
		prey = (bloc_t *)sbrk( expended );
		prey->size = expended - sizeof(bloc_t);
		prey->next = NULL;
		return (void *)(prey + 1);
	}
	if( !count )
	{
		size = table_size[idx];
		prey = (bloc_t *)sbrk( sizeof(bloc_t) + size );
		prey->size = size;
		prey->next = NULL;
		return (void *)(prey + 1);
	}
	if( idx == NUM_SIZES - 1 )
	{
		bloc_t * reserve = flist[idx]->payload;
		prey = reserve->next;
		
		if(reserve->size >= size)
		{
			flist[idx]->payload = prey;
			flist[idx]->num--;
			return (void *)(reserve + 1);
		}
		while( prey != NULL)
		{
			if(prey->size >= size)
			{
				reserve->next = prey->next;
				flist[idx]->num--;
				return (void *)(prey + 1);
			}
			reserve = prey;
			prey = prey->next;
		}
	
		size_t expended = align(size);
		prey = (bloc_t *)sbrk( expended );
		prey->size = expended - sizeof(bloc_t);
		prey->next = NULL;
		flist[idx]->num--;
		return (void *)(prey + 1);
	}
	prey = flist[idx]->payload;
	flist[idx]->payload = prey->next;
	flist[idx]->num--;

	return (void *)(prey + 1);
}

void *calloc(size_t num, size_t size)
{
	/* Note: This function is complete. You do not need to modify it. */
	void *ptr = malloc(num * size);
	
	if (ptr) {
		memset(ptr, 0x00, num * size);
	}

	return ptr;
}

void * malloc(size_t size)
{
	//printf("new size: %lu \n", size);
	if(iflag)
	{
		init();
		iflag = 0;
	}
	return hunt(size);
}

void free(void *ptr)
{
	if (!ptr)
		return;

	bloc_t * b = (bloc_t *)(ptr - sizeof(bloc_t));

	int idx = find_idx( b->size );

	b->next = flist[idx]->payload;
	flist[idx]->payload = b;

	flist[idx]->num++;
	return;
}

void *realloc(void *ptr, size_t size) {
	if (!ptr)
		return malloc(size);

	if (!size)
	{
		free(ptr);
		return NULL;
	}

	bloc_t * b = (bloc_t *)(ptr - sizeof(bloc_t));

	double percent = ( (double)size / b->size );

	if(!big)
	{
		if( percent < 0.9 )
		{
			if( b->size - sizeof(bloc_t) > 536870912 )
			{
				return split(b, size);
			}
			return ptr;
		}
		if( percent <= 1.0 )
		{
			return ptr;
		}

		void *neo = malloc(size);
		memcpy(neo, ptr, b->size);
		free(ptr);
		return neo;
	}
	else
	{
		if( percent <0.4 )
		{
			return bs(b);
		}
		if(percent < 1.0)
		{
			return ptr;
		}
		void *neo = malloc(size);
		memcpy(neo, ptr, b->size);
		free(ptr);
		return neo;
	}
}
