/******Abygail Stiekman*******/
/*****11/20/2016 CDA3101******/
/**Proj 3 - Cache Simulator***/
/***********aes15d************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>


typedef struct block_t{
	int tag;
	int lru;
	int dirty;
	int valid;
}block_t;

int main(int argc, char *argv[])
{//keep track of data
	 unsigned int offset;
	 unsigned int index;
	 unsigned int tag;
	char block_size[3];
	char num_sets[3];
	char associativity[3];
	int i;
//check for command line arguments
//set them to vars(block_size, num_sets, associativity)
	for(i=1; i<argc; i++)
	{
		if(strcmp("-b", argv[i]) == 0)
			strcpy(block_size, argv[i+1]);

		else if(strcmp("-s", argv[i]) == 0)
			strcpy(num_sets, argv[i+1]);

		else if(strcmp("-n", argv[i]) == 0)
			strcpy(associativity, argv[i+1]);
	}
//performs atoi
	int blockSize = atoi(block_size);
	int numSets = atoi(num_sets); 
	int assoc = atoi(associativity);

	//prints data post aoi
	printf("%s %d\n", "Block Size: ", blockSize);
	printf("%s %d\n", "Number of Sets: ",  numSets);
	printf("%s %d\n", "Associativity: ",  assoc);

	offset = log2(blockSize);
	index = log2(numSets);
//offset = (log2(blockSize) = log10(blockSize)/log10(2));
//index = (log2(numSets) = log10(numSets)/log10(2));

	printf("%s %d\n", "Number of offset bits: ",  offset);

	printf("%s %d\n", "Number of index bits: ",  index);

	tag = 32 - offset;
	tag = tag - index;

	printf("%s %d\n", "Number of tag bits: ", tag);

//creates structures for wt and writeback caches
	struct block_t WT_cache[numSets][assoc];
	struct block_t WB_cache[numSets][assoc];


	char access_type;
	unsigned int byte_address;
	char instr[50];
	int tagBits;
	int indexBits;
//goes through the # of sets and 
//sets all of the 2d array variables to 0
	int j = 0;
	for(i = 0; i<numSets; i++){
		for(j = 0; j<assoc; j++)
		{
			WT_cache[i][j].dirty = 0;
			WB_cache[i][j].dirty = 0;
			WT_cache[i][j].valid = 0;
			WB_cache[i][j].valid = 0;
			WT_cache[i][j].tag = 0;
			WB_cache[i][j].tag = 0;
			WT_cache[i][j].lru = 0;
			WB_cache[i][j].lru = 0;
		}
	}

	int WT_hits = 0;
	int WB_hits = 0;
	int WT_misses = 0;
	int WB_misses = 0;
	int WT_total_ref = 0;
	int WB_total_ref = 0;
	int WT_memory_ref = 0;
	int WB_memory_ref = 0;
	int WB_hitBool = 0;
	int WT_missBool = 0;
	int WB_missBool = 0;
	int hold = 0;
	int WT_hitBool = 0;
	int hold_index = 0;

	while(fgets(instr, 100, stdin) != NULL)
	{
		  WT_hitBool = 0;
        	  WB_hitBool = 0;
		  i = 0;
		  hold = 0;
		  hold_index = 0;
		  //WT_memory_reference++;
			//WB_memory_reference++;
		  WT_total_ref++;
		  WB_total_ref++;

	/*--------------------- read - wt and wb ----------------------------*/
		if (sscanf(instr, "R %d", &byte_address) ==1)
		{
			hold = 0;
			hold_index = 0;
			//finds tag
			tagBits = byte_address >> (32-tag);                 
			indexBits =(byte_address >> offset)% numSets;


		/*------------------- wt - no write alloc ---------------- */
			WT_hitBool = 0;
			WB_hitBool = 0;

			for(i=0; i<assoc; i++)
			{
				if(WT_cache[indexBits][i].lru > hold)
				{
					hold_index=i;
					hold=WT_cache[indexBits][i].lru;
				}

			}

			for(j=0; j<assoc; j++)
			{
				if(WT_cache[indexBits][j].valid == 1)
				{
					if(WT_cache[indexBits][j].tag == tagBits)
					{
						WT_hits++;
						WT_cache[indexBits][j].lru = 0;
						WT_hitBool=1;

					}
					else
					{
						WT_missBool=1;
						WT_cache[indexBits][j].lru+=1;

					}
				}
				else if(WT_cache[indexBits][j].valid == 0)
				{
						WT_missBool=1;
						WT_cache[indexBits][j].lru+=1;
				}

				if(WT_hitBool==0 && j==(assoc-1))
				{
					WT_memory_ref++;
					WT_misses++;
					WT_cache[indexBits][hold_index].lru=0;
					WT_cache[indexBits][hold_index].valid=1;
					WT_cache[indexBits][hold_index].tag=tagBits;
				}
			}


		/* ------------------------ wb - write alloc read -------------------*/
			for(i=0; i<assoc; i++)
			{
				if(WB_cache[indexBits][i].lru > hold)
				{
					hold_index=i;
					hold=WB_cache[indexBits][i].lru;
				}
			}

			for(j=0; j<assoc; j++)
			{
				if(WB_cache[indexBits][j].valid == 1)
                                {
                                        if(WB_cache[indexBits][j].tag==tagBits)
                                        {
                                                WB_hits++;
                                                WB_cache[indexBits][j].lru=0;
                                                WB_hitBool=1;

                                        }
                                        else
                                        {
                                                WB_missBool=1;
                                                WB_cache[indexBits][j].lru+=1;

                                        }
                                }
				else if(WB_cache[indexBits][j].valid == 0)
				{
					 	WB_missBool=1;
                        WB_cache[indexBits][j].lru+=1;
				}

                    if(WB_hitBool==0 && j==(assoc-1))
                    {
						WB_misses++;
						WB_memory_ref++;
                        WB_cache[indexBits][hold_index].lru=0;
                        WB_cache[indexBits][hold_index].valid=1;
						WB_cache[indexBits][hold_index].tag=tagBits;
					if(WB_cache[indexBits][hold_index].dirty==1)
						{
							WB_memory_ref++;
						}
						WB_cache[indexBits][hold_index].dirty=0;
                    }
            }


		}
	/* ---------------------- wt - no write alloc write --------------------------- */

		else if (sscanf(instr, "W %d", &byte_address) == 1)
		{
			WT_hitBool = 0;
			WB_hitBool = 0;
			hold = 0;
			hold_index = 0;
			WT_memory_ref++;

			tagBits = byte_address >> (32-tag);                 //findTag
			indexBits =(byte_address >> offset)% numSets;

			for(i=0; i<assoc; i++)
			{
				 if(WT_cache[indexBits][i].lru > hold)
                                {
                                        hold_index=i;
                                        hold=WT_cache[indexBits][i].lru;
                                }

			}


			for(j=0; j<assoc; j++)
			{
                                if(WT_cache[indexBits][j].valid == 1)
                                {
                                        if(WT_cache[indexBits][j].tag==tagBits)
                                        {
                                                WT_hits++;
                                                WT_cache[indexBits][j].lru=0;
                                                WT_hitBool=1;
					 }
                                        else
                                        {
                                                WT_missBool=1;
                                                WT_cache[indexBits][j].lru+=1;

                                        }
                                }

				else if(WT_cache[indexBits][j].valid == 0)
				{
						WT_missBool=1;
                        WT_cache[indexBits][j].lru+=1;
				}
                 if(WT_hitBool==0 && j==(assoc-1))
                    WT_misses++;
			}

		 /* ---------------------- wb - write alloc write --------------------------- */

			for(i=0; i<assoc; i++)
                {
                    if(WB_cache[indexBits][i].lru > hold)
                    {
                        hold_index=i;
                        hold=WT_cache[indexBits][i].lru;
                    }
                }
                        for(j = 0; j<assoc; j++)
                        {
                                if(WB_cache[indexBits][j].valid == 1)
                                {
                                        if(WB_cache[indexBits][j].tag==tagBits)
                                        {
                                                WB_hits++;
                                                WB_cache[indexBits][j].lru = 0;
                                                WB_hitBool=1;
                                                WB_cache[indexBits][j].dirty = 1;
                                         }
                                        else
                                        {
												WB_missBool = 1;
                                                WB_cache[indexBits][j].lru+=1;

                                        }
                                }
                                else if(WB_cache[indexBits][j].valid == 0)
                                {
												WB_missBool = 1;
                                                WB_cache[indexBits][j].lru+=1;
								}
                                if(WB_hitBool == 0 && j==(assoc-1))
                                {
										WB_misses++;
										WB_memory_ref++;
                                        WB_cache[indexBits][hold_index].lru = 0;
                                        WB_cache[indexBits][hold_index].valid = 1;
										WB_cache[indexBits][hold_index].tag=tagBits;
											if(WB_cache[indexBits][hold_index].dirty == 1)
											{
												WB_memory_ref+=1;
											}
										WB_cache[indexBits][hold_index].dirty = 1;
								}

                        }



		}
	}
//prints data for both write through and write back
	puts("\n");
	puts("****************************************");
	puts("Write-through with No Write Allocate");
	puts("****************************************");
	printf("%s %d\n", "Total number of references: ", WT_total_ref);
	printf("%s %d\n", "Hits: ", WT_hits);
	printf("%s %d\n", "Misses: ", WT_misses);
	printf("%s %d\n", "Memory references: ", WT_memory_ref);


		puts("\n");
        puts("****************************************");
        puts("Write-Back Write Allocate");
        puts("****************************************");
        printf("%s %d\n", "Total number of references: ", WB_total_ref);
        printf("%s %d\n", "Hits: ", WB_hits);
        printf("%s %d\n", "Misses: ", WB_misses);
        printf("%s %d\n", "Memory references: ", WB_memory_ref);
		
		
		
	return 0;
}