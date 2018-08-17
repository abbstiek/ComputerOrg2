//Abygail Stiekman
//09/23/2016

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>


// Struct that stores registers and their respective binary reference
struct {
	const char *name;
	char *add;
} regmap[] = {
		{ "zero", "00000" },
		{ "t0", "01000" },
		{ "t1", "01001" },
		{ "t2", "01010" },
		{ "t3", "01011" },
		{ "t4", "01100" },
		{ "t5", "01101" },
		{ "t6", "01110" },
		{ "t7", "01111" },
		{ "s0", "10000" },
		{ "s1", "10001" },
		{ "s2", "10010" },
		{ "s3", "10011" },
		{ "s4", "10100" },
		{ "s5", "10101" },
		{ "s6", "10110" },
		{ "s7", "10111" },
		{ NULL, 0 } };

// R -type instructions
struct {
	const char *name;
	char *function;
} rMap[] = {
		{ "add", "100000" },
		{ "sll", "000000" },
		{ "nor", "100111" },
		{ NULL, 0 } };

// Struct for I-Type instructions
struct {
	const char *name;
	char *add;
} imap[] = {
		{ "lw",   "100011" },
		{ "sw",   "101011" },
		{ "ori",  "001101" },
		{ "lui",  "001111" },
		{ "addi", "001000" },
		{"bne",	"000101"},
		{ NULL, 0 } };

// Struct for J-Type instructions
struct {
	const char *name;
	char *add;
} jmap[] = {
		{ "j", "000010" },
		{ NULL, 0 } };

	int search(char *instr);

// array of instructions included in assembler
char *instr[] = {
		"la",
		"lui",	
		"lw",	
		"sw",	
		"add",	
		"addi",
		"ori",
		"sll",
		"bne",
		"j",
		"nor"
	};

// Size of array
size_t inst_len = sizeof(instr)/sizeof(char *);

int search(char *instr) {

	int found = 0;
	int i;
	for (i = 0; i < inst_len; i++) {

		if (strcmp(instr, instr[i]) == 0) {
			found = 1;
			return i;
		}
	}

	if (found == 0)
		return -1;
}

//comparison func
int string_comp(const void *a, const void *b) {
	return strcmp(*(char **)a, *(char **)b);
}	
		
void getBin(int num, char *string, int wow) {

	*(string + wow) = '\0';

	long pos;
	if (wow == 5)
		pos = 0x10;
	else if (wow == 16)
		pos = 0x8000;
	else if (wow == 26)
		pos = 0x2000000;
	else if (wow == 32)
		pos = 0x80000000;

	long mask = pos << 1;
	while (mask >>= 1)
		*string++ = !!(mask & num) + '0';
}

void file_pars(FILE *fptr, int pass, char *instr[], size_t inst_len, hash_table_t *hash_table, FILE *Out) {

	char line[MAX_LINE_LENGTH + 1];
	char *tok_ptr, *ret, *token = NULL;
	int32_t line_num = 1;
	int32_t instruction_count = 0x00000000;
	int data_reached = 0;

	while (1) {
		if ((ret = fgets(line, MAX_LINE_LENGTH, fptr)) == NULL)
			break;
		line[MAX_LINE_LENGTH] = 0;

		tok_ptr = line;
		if (strlen(line) == MAX_LINE_LENGTH) {
			fprintf(Out,
					"line %d: line is too long. ignoring line ...\n", line_num);
			line_num++;
			continue;
		}

		//parse token in line
		while (1) {

			token = parse_token(tok_ptr, " \n\t$,", &tok_ptr, NULL);

			//go to next line
			if (token == NULL || *token == '#') {
				line_num++;
				free(token);
				break;
			}

			printf("token: %s\n", token);

			 // If token is "la", increment by 8, otherwise if it exists in instructions[],
			 // increment by 4
			int x = search(token);
			if (x >= 0) {
				if (strcmp(token, "la") == 0)
					instruction_count = instruction_count + 8;
				else
					instruction_count = instruction_count + 4;
			}

			// If token is ".data", reset instruction to .data starting address
			else if (strcmp(token, ".data") == 0) {
				instruction_count = 0x00002000;
				data_reached = 1;
			}

			printf("PC Count: %d\n", instruction_count);

			// If first pass, then add labels to hash table
			if (pass == 1) {

				printf("First pass\n");
				// : = label
				if (strstr(token, ":") && data_reached == 0) {

					printf("Label\n");

					size_t token_len = strlen(token);
					token[token_len - 1] = '\0';

					// Insert variable to hash table
					uint32_t *inst_count;
					inst_count = (uint32_t *)malloc(sizeof(uint32_t));
					*inst_count = instruction_count;
					int32_t insert = hash_insert(hash_table, token, strlen(token)+1, inst_count);

					if (insert != 1) {
						fprintf(Out, "Error inserting into hash table\n");
						exit(1);
					}
				}

				// If .data has been reached, increment instruction count accordingly
				// and store to hash table
				else {

					char *var_tok = NULL;
					char *var_tok_ptr = tok_ptr;

					// If variable is .word
					if (strstr(tok_ptr, ".word")) {

						printf(".word\n");

						// Variable is array
						if (strstr(var_tok_ptr, ":")) {

							printf("array\n");

							// Store the number in var_tok and the occurance in var_tok_ptr
							var_tok = parse_token(var_tok_ptr, ":", &var_tok_ptr, NULL);

							// Convert char* to int
							int freq = atoi(var_tok_ptr);

							int num;
							sscanf(var_tok, "%*s %d", &num);

							// Increment instruction count by freq
							instruction_count = instruction_count + (freq * 4);

							// Strip out ':' from token
							size_t token_len = strlen(token);
							token[token_len - 1] = '\0';

							//printf("Key: '%s', len: %zd\n", token, strlen(token));

							// Insert variable to hash table
							uint32_t *inst_count;
							inst_count = (uint32_t *)malloc(sizeof(uint32_t));
							*inst_count = instruction_count;
							int32_t insert = hash_insert(hash_table, token, strlen(token)+1, inst_count);

							if (insert == 0) {
								fprintf(Out, "Error in hash table insertion\n");
								exit(1);
							}

							printf("End array\n");
						}

						// Variable is a single variable
						else {

							instruction_count = instruction_count + 4;

							// Strip out ':' from token
							size_t token_len = strlen(token);
							token[token_len - 1] = '\0';

							// Insert variable to hash table
							uint32_t *inst_count;
							inst_count = (uint32_t *)malloc(sizeof(uint32_t));
							*inst_count = instruction_count;
							int32_t insert = hash_insert(hash_table, token, strlen(token)+1, inst_count);

							if (insert == 0) {
								fprintf(Out, "Error in hash table insertion\n");
								exit(1);
							}

							printf("end singe var\n");
						}
					}

					// Variable is a string
					else if (strstr(tok_ptr, ".asciiz")) {

						// Store the ascii in var_tok
						var_tok_ptr+= 8;
						var_tok = parse_token(var_tok_ptr, "\"", &var_tok_ptr, NULL);

						// Increment instruction count by string length
						size_t str_byte_len = strlen(var_tok);
						instruction_count = instruction_count + str_byte_len;

						// Strip out ':' from token
						size_t token_len = strlen(token);
						token[token_len - 1] = '\0';

						// Insert variable to hash table
						uint32_t *inst_count;
						inst_count = (uint32_t *)malloc(sizeof(uint32_t));
						*inst_count = instruction_count;
						int32_t insert = hash_insert(hash_table, token, strlen(token)+1, inst_count);

						if (insert == 0) {
							fprintf(Out, "Error in hash table insertion\n");
							exit(1);
						}
					}
				}
			}

			// If second pass, then interpret
			else if (pass == 2) {

				printf("############    Pass 2   ##############\n");

				// if j loop --> then instruction is: 000010
				
				// .text
				if (data_reached == 0) {

					// Check instruction type
					int instruction_supported = search(token);
					char inst_type;

					// If instruction is supported
					if (instruction_supported != -1) {

						// token contains the instruction
						// tok_ptr points to the rest of the line

						// Determine instruction type
						inst_type = instr_type(token);

						if (inst_type == 'r') {

							//rtype
							if (strcmp(token, "add") == 0) {

								// Parse the instructio - get rd, rs, rt registers
								char *inst_ptr = tok_ptr;
								char *reg = NULL;

								// array stores rs, rt, rd respectively
								char **reg_store;
								reg_store = malloc(3 * sizeof(char*));
								if (reg_store == NULL) {
									fprintf(Out, "Out of memory\n");
									exit(1);
								}
								int i;
								for (i = 0; i < 3; i++) {
									reg_store[i] = malloc(2 * sizeof(char));
									if (reg_store[i] == NULL) {
										fprintf(Out, "Out of memory\n");
										exit(1);
									}
								}

								// which register has been parsed for storage
								int count = 0;
								while (1) {

									reg = parse_token(inst_ptr, " $,\n\t", &inst_ptr, NULL);

									if (reg == NULL || *reg == '#') {
										break;
									}

									strcpy(reg_store[count], reg);
									count++;
									free(reg);
								}

								// Send reg_store for output
								// rd is in position 0, rs is in position 1 and rt is in position 2
								rtype_instr(token, reg_store[1], reg_store[2], reg_store[0], 0, Out);
								int i;
								// Dealloc reg_store
								for (i = 0; i < 3; i++) {
									free(reg_store[i]);
								}
								free(reg_store);
							}

							// R-Type with $rd, $rs, shamt format
							else if (strcmp(token, "sll") == 0 || strcmp(token, "srl") == 0) {

								// Parse the instructio - get rd, rs, rt registers
								char *inst_ptr = tok_ptr;
								char *reg = NULL;

								// Create an array of char* that stores rd, rs and shamt
								char **reg_store;
								reg_store = malloc(3 * sizeof(char*));
								if (reg_store == NULL) {
									fprintf(Out, "Out of memory\n");
									exit(1);
								}
								int i;
								for (i = 0; i < 3; i++) {
									reg_store[i] = malloc(2 * sizeof(char));
									if (reg_store[i] == NULL) {
										fprintf(Out, "Out of memory\n");
										exit(1);
									}
								}

								// Keeps a reference to which register has been parsed for storage
								int count = 0;
								while (1) {

									reg = parse_token(inst_ptr, " $,\n\t", &inst_ptr, NULL);

									if (reg == NULL || *reg == '#') {
										break;
									}

									strcpy(reg_store[count], reg);
									count++;
									free(reg);
								}

								// Send reg_store for output
								// rd is in position 0, rs is in position 1 and shamt is in position 2
								rtype_instr(token, "00000", reg_store[1], reg_store[0], atoi(reg_store[2]), Out);
								int i;
								// Dealloc reg_store
								for (i = 0; i < 3; i++) {
									free(reg_store[i]);
								}
								free(reg_store);
							}

							else if (strcmp(token, "jr") == 0) {

								// Parse the instruction - rs is in tok_ptr
								char *inst_ptr = tok_ptr;
								char *reg = NULL;
								reg = parse_token(inst_ptr, " $,\n\t", &inst_ptr, NULL);

								rtype_instr(token, reg, "00000", "00000", 0, Out);
							}
						}

						// I-Type
						else if (inst_type == 'i') {

							// la is pseudo instruction for lui and ori
							// Convert to lui and ori and pass those instructions
							if (strcmp(token, "la") == 0) {

								// Parse the instruction - get register & immediate
								char *inst_ptr = tok_ptr;
								char *reg = NULL;

								// Create an array of char* that stores rd, rs and shamt
								char **reg_store;
								reg_store = malloc(2 * sizeof(char*));
								if (reg_store == NULL) {
									fprintf(Out, "Out of memory\n");
									exit(1);
								}
								int i;
								for (i = 0; i < 2; i++) {
									reg_store[i] = malloc(2 * sizeof(char));
									if (reg_store[i] == NULL) {
										fprintf(Out, "Out of memory\n");
										exit(1);
									}
								}

								// Keeps a reference to which register has been parsed for storage
								int count = 0;
								while (1) {

									reg = parse_token(inst_ptr, " $,\n\t", &inst_ptr, NULL);

									if (reg == NULL || *reg == '#') {
										break;
									}

									strcpy(reg_store[count], reg);
									count++;
									free(reg);
								}

								// Interpret la instruction.
								// The register is at reg_store[0] and the variable is at reg_store[1]

								// Find address of label in hash table
								int *add = hash_find(hash_table, reg_store[1], strlen(reg_store[1])+1);

								// Convert address to binary in char*
								char addressBinary[33];
								getBin(*address, addressBinary, 32);

								// Get upper and lower bits of address
								char upperBits[16];
								char lowerBits[16];
								int i;
								for (i = 0; i < 32; i++) {
									if (i < 16)
										lowerBits[i] = addressBinary[i];
									else
										upperBits[i-16] = addressBinary[i];
								}

								// Call the lui instruction with: lui $reg, upperBits
								// Convert upperBits binary to int
								int immediate = getDec(upperBits);
								itype_instr("lui", "00000", reg_store[0], immediate, Out);

								// Call the ori instruction with: ori $reg, $reg, lowerBits
								// Convert lowerBits binary to int
								immediate = getDec(lowerBits);
								itype_instr("ori", reg_store[0], reg_store[0], immediate, Out);
								int i;
								// Dealloc reg_store
								for (i = 0; i < 2; i++) {
									free(reg_store[i]);
								}
								free(reg_store);
							}

							// I-Type $rt, i($rs)
							else if (strcmp(token, "lw") == 0 || strcmp(token, "sw") == 0) {

								// Parse the instructio - rt, immediate and rs
								char *inst_ptr = tok_ptr;
								char *reg = NULL;
								//
								// Create an array of char* that stores rd, rs, rt respectively
								char **reg_store;
								reg_store = malloc(3 * sizeof(char*));
								if (reg_store == NULL) {
									fprintf(Out, "Out of memory\n");
									exit(1);
								}
								int i;
								for (i = 0; i < 3; i++) {
									reg_store[i] = malloc(2 * sizeof(char));
									if (reg_store[i] == NULL) {
										fprintf(Out, "Out of memory\n");
										exit(1);
									}
								}

								// Keeps a reference to which register has been parsed for storage
								int count = 0;
								while (1) {

									reg = parse_token(inst_ptr, " $,\n\t()", &inst_ptr, NULL);

									if (reg == NULL || *reg == '#') {
										break;
									}

									strcpy(reg_store[count], reg);
									count++;
									free(reg);
								}

								// rt in position 0, immediate in position 1 and rs in position2
								int immediate = atoi(reg_store[1]);
								itype_instr(token, reg_store[2], reg_store[0], immediate, Out);
								int i;
								// Dealloc reg_store
								for (i = 0; i < 3; i++) {
									free(reg_store[i]);
								}
								free(reg_store);
							}

							// I-Type rt, rs, im
							else if (strcmp( token, "ori") == 0 || strcmp(token, "slti") == 0 || strcmp(token, "addi") == 0) {

								// Parse the instruction - rt, rs, immediate
								char *inst_ptr = tok_ptr;
								char *reg = NULL;

								// Create an array of char* that stores rt, rs
								char **reg_store;
								reg_store = malloc(3 * sizeof(char*));
								if (reg_store == NULL) {
									fprintf(Out, "Out of memory\n");
									exit(1);
								}
								int i;
								for (i = 0; i < 3; i++) {
									reg_store[i] = malloc(2 * sizeof(char));
									if (reg_store[i] == NULL) {
										fprintf(Out, "Out of memory\n");
										exit(1);
									}
								}

								// Keeps a reference to which register has been parsed for storage
								int count = 0;
								while (1) {

									reg = parse_token(inst_ptr, " $,\n\t", &inst_ptr, NULL);

									if (reg == NULL || *reg == '#') {
										break;
									}

									strcpy(reg_store[count], reg);
									count++;
									free(reg);
								}

								// rt in position 0, rs in position 1 and immediate in position 2
								int immediate = atoi(reg_store[2]);
								itype_instr(token, reg_store[1], reg_store[0], immediate, Out);
								int i;
								// Dealloc reg_store
								for (i = 0; i < 3; i++) {
									free(reg_store[i]);
								}
								free(reg_store);
							}

							// I-Type $rt, immediate
							else if (strcmp(token, "lui") == 0) {

								// Parse the insturction,  rt - immediate
								char *inst_ptr = tok_ptr;
								char *reg = NULL;

								// Create an array of char* that stores rs, rt
								char **reg_store;
								reg_store = malloc(2 * sizeof(char*));
								if (reg_store == NULL) {
									fprintf(Out, "Out of memory\n");
									exit(1);
								}
								int i;
								for (i = 0; i < 2; i++) {
									reg_store[i] = malloc(2 * sizeof(char));
									if (reg_store[i] == NULL) {
										fprintf(Out, "Out of memory\n");
										exit(1);
									}
								}

								// Keeps a reference to which register has been parsed for storage
								int count = 0;
								while (1) {

									reg = parse_token(inst_ptr, " $,\n\t", &inst_ptr, NULL);

									if (reg == NULL || *reg == '#') {
										break;
									}

									strcpy(reg_store[count], reg);
									count++;
									free(reg);
								}
								int immediate = atoi(reg_store[1]);
								itype_instr(token, "00000", reg_store[0], immediate, Out);
								int i;
								// deallocate
								for (i = 0; i < 3; i++) {
									free(reg_store[i]);
								}
								free(reg_store);
							}

							// I-Type $rs, $rt, label
							else if (strcmp(token, "bne") == 0) {

								// Parse the instruction - rs, rt
								char *inst_ptr = tok_ptr;
								char *reg = NULL;

								// Create an array of char* that stores rs, rt
								char **reg_store;
								reg_store = malloc(2 * sizeof(char*));
								if (reg_store == NULL) {
									fprintf(Out, "Out of memory\n");
									exit(1);
								}
								int i;
								for (i = 0; i < 2; i++) {
									reg_store[i] = malloc(2 * sizeof(char));
									if (reg_store[i] == NULL) {
										fprintf(Out, "Out of memory\n");
										exit(1);
									}
								}

								// Keeps a reference to which register has been parsed for storage
								int count = 0;
								while (1) {

									reg = parse_token(inst_ptr, " $,\n\t", &inst_ptr, NULL);

									if (reg == NULL || *reg == '#') {
										break;
									}

									strcpy(reg_store[count], reg);
									count++;
									free(reg);

									if (count == 2)
										break;
								}

								reg = parse_token(inst_ptr, " $,\n\t", &inst_ptr, NULL);

								// Find hash address for a register and put in an immediate
								int *add = hash_find(hash_table, reg, strlen(reg)+1);
								
								int immediate = *add + instruction_count;

								// Send instruction to itype function
								itype_instr(token, reg_store[0], reg_store[1], immediate, Out);
								int i;
								// Dealloc reg_store
								for (i = 0; i < 2; i++) {
									free(reg_store[i]);
								}
								free(reg_store);
							}
						}

						// J-Type
						else if (inst_type == 'j') {

							// Parse the instruction - get label
							char *inst_ptr = tok_ptr;

							// If comment, extract the label alone
							char *comment = strchr(inst_ptr, '#');
							if (comment != NULL) {							
								int i;
								int str_len_count = 0;
								for (i = 0; i < strlen(inst_ptr); i++) {
									if (inst_ptr[i] != ' ')
										str_len_count++;
									else
										break;
								}
								int i;
								char new_label[str_len_count+1];
								for (i = 0; i < str_len_count; i++)
									new_label[i] = inst_ptr[i];
								new_label[str_len_count] = '\0';

								strcpy(inst_ptr, new_label);
							}

							else { printf("NO COMMENT\n");
								inst_ptr[strlen(inst_ptr)-1] = '\0'; 
							}

							// Find hash address for a label and put in an immediate
							int *add = hash_find(hash_table, inst_ptr, strlen(inst_ptr)+1);
							
							// Send to jtype function
							jtype_instr(token, *add, Out);
						}
					}

					if (strcmp(token, "nop") == 0) {
						fprintf(Out, "00000000000000000000000000000000\n");
					}
				}

				// If .data part reached
				else {

					char *var_tok = NULL;
					char *var_tok_ptr = tok_ptr;

					// If variable is .word
					if (strstr(tok_ptr, ".word")) {

						int var_value;

						// Variable is array
						if (strstr(var_tok_ptr, ":")) {

							// Store the number in var_tok and the occurance in var_tok_ptr
							var_tok = parse_token(var_tok_ptr, ":", &var_tok_ptr, NULL);

							// Extract array size, or variable frequency
							int freq = atoi(var_tok_ptr);

							// Extract variable value
							sscanf(var_tok, "%*s %d", &var_value);
							int i;
							// Value var_value is repeated freq times. Send to binary rep function
							for (i = 0; i < freq; i++) {
								word_rep(var_value, Out);
							}
						}

						// Variable is a single variable
						else {

							// Extract variable value
							sscanf(var_tok_ptr, "%*s, %d", &var_value);

							// Variable is in var_value. Send to binary rep function
							word_rep(var_value, Out);
						}
					}

					// Variable is a string
					else if (strstr(tok_ptr, ".asciiz")) {

						printf("tok_ptr '%s'\n", tok_ptr);

						if (strncmp(".asciiz ", var_tok_ptr, 8) == 0) {

							// Move var_tok_ptr to beginning of string
							var_tok_ptr = var_tok_ptr + 9;

							// Strip out quotation at the end
							// Place string in var_tok
							var_tok = parse_token(var_tok_ptr, "\"", &var_tok_ptr, NULL);

							ascii_rep(var_tok, Out);
						}
					}
				}
			}

			free(token);
		}
	}
}

// Binary Search the Array
int binarySearch(char *instr[], int low, int high, char *string) {

	int mid = low + (high - low) / 2;
	int comp = strcmp(instr[mid], string);\

	if (comp == 0)
		return mid;

	// prints back if not found
	if (high <= low)
		return -1;

	//if array is smaller than string
	else if (comp > 0)
		return binarySearch(instr, low, mid - 1, string);

	// if array is smaller than string
	else if (comp < 0)
		return binarySearch(instr, mid + 1, high, string);

	// Return position
	else
		return mid;

	// Error
	return -2;
}

// Determine Instruction Type
char instr_type(char *instr) {

	if (strcmp(instruction, "add") == 0 || strcmp(instruction, "sll") == 0 ||
			 strcmp(instruction, "nor") == 0) {

		return 'r';
	}

	else if (strcmp(instruction, "lw") == 0 || strcmp(instruction, "sw") == 0
			|| strcmp(instruction, "ori")== 0 || strcmp(instruction, "lui") == 0 
			|| strcmp(instruction, "bne") == 0 || strcmp(instruction, "addi") == 0 
			|| strcmp(instruction, "la") == 0) {

		return 'i';
	}

	else if (strcmp(instr, "j") == 0)) 
	{
		return 'j';
	}

	// Failsafe return statement
	return 0;
}

// Return the binary representation of the register
char *register_address(char *registerName) {

	size_t i;
	for (i = 0; regmap[i].name != NULL; i++) {
		if (strcmp(registerName, regmap[i].name) == 0) {
			return regmap[i].addr;
		}
	}

	return NULL;
}

// Write out the R-Type instruction
void rtype_instr(char *instr, char *rs, char *rt, char *rd, int shamt, FILE *Out) {

	// Set the instruction bits
	char *opcode = "000000";

	char *rd_Bin = "00000";
	if (strcmp(rd, "00000") != 0)
		rd_Bin = register_address(rd);

	char *rs_Bin = "00000";
	if (strcmp(rs, "00000") != 0)
		rs_Bin = register_address(rs);

	char *rt_Bin = "00000";
	if (strcmp(rt, "00000") != 0)
		rt_Bin = register_address(rt);

	char *func = NULL;
	char shamtBin[6];

	// Convert shamt to binary and put in shamtBin as a char*
	getBin(shamt, shamtBin, 5);

	size_t i;
	for (i = 0; rMap[i].name != NULL; i++) {
		if (strcmp(instr, rMap[i].name) == 0) {
			func = rMap[i].function;
		}
	}

	// Print out the instruction to the file
	fprintf(Out, "%s%s%s%s%s%s\n", opcode, rs_Bin, rt_Bin, rd_Bin, shamtBin, func);
}

// Write out the I-Type instruction
void itype_instr(char *instr, char *rs, char *rt, int immediateNum, FILE *Out) {

	// Set the instruction bits
	char *rs_Bin = "00000";
	if (strcmp(rs, "00000") != 0)
		rs_Bin = register_address(rs);

	char *rt_Bin = "00000";
		if (strcmp(rt, "00000") != 0)
			rt_Bin = register_address(rt);

	char *opcode = NULL;
	char immediate[17];

	size_t i;
	for (i = 0; imap[i].name != NULL; i++) {
		if (strcmp(instr, imap[i].name) == 0) {
			opcode = imap[i].addr;
		}
	}

	// Convert immediate to binary
	getBin(immediateNum, immediate, 16);

	// Print out the instruction to the file
	fprintf(Out, "%s%s%s%s\n", opcode, rs_Bin, rt_Bin, immediate);
}

// Write out the J-Type instruction
void jtype_instr(char *instr, int immediate, FILE *Out) {

	// Set the instruction bits
	char *opcode = NULL;

	// Get opcode bits
	size_t i;
	for (i = 0; jmap[i].name != NULL; i++) {
		if (strcmp(instr, jmap[i].name) == 0) {
			opcode = jmap[i].addr;
		}
	}

	// Convert immediate to binary
	char immediateStr[27];
	getBin(immediate, immediateStr, 26);

	// Print out instruction to file
	fprintf(Out, "%s%s\n", opcode, immediateStr);
}

// Write out the variable in binary
void word_rep(int binary_rep, FILE *Out) {
	int k;
	for (k = 31; k >= 0; k--) {
		fprintf(Out, "%c", (binary_rep & (1 << k)) ? '1' : '0');
	}

	fprintf(Out, "\n");
}

// Write out the ascii string
void ascii_rep(char string[], FILE *Out) {

	// Separate the string, and put each four characters in an element of an array of strings
	size_t str_length = strlen(string);
	str_length++;
	int num_strs = str_length / 4;
	if ((str_length % 4) > 0)
		num_strs++;

	char *ptr;
	ptr = &string[0];

	// Create an array of strings which separates each 4-char string
	char **strins;
	strins = malloc(num_strs * sizeof(char*));
	if (strins == NULL) {
		fprintf(Out, "Out of memory\n");
		exit(1);
	}
	int i;
	for (i = 0; i < num_strs; i++) {
		strins[i] = malloc(4 * sizeof(char));
		if (strins[i] == NULL) {
			fprintf(Out, "Out of memory\n");
			exit(1);
		}
	}
	int i;
	int count = 0;
	for (i = 0; i < str_length; i++) {
		strins[i / 4][i % 4] = *ptr;
		ptr++;
		count++;
	}

	// Reverse each element in the array
	char temp;
	
	for (i = 0; i < num_strs; i++) {
	int j;
		for (j = 0, k = 3; j < k; j++, k--) {

			temp = strins[i][j];
			strins[i][j] = strins[i][k];
			strins[i][k] = temp;
		}
	}
	// Convert into binary
	for (i = 0; i < num_strs; i++) {

		for (int j = 0; j < 4; j++) {
			char c = strins[i][j];
			int k;
			for (k = 7; k >= 0; k--) {
				fprintf(Out, "%c", (c & (1 << k)) ? '1' : '0');
			}
		}

		fprintf(Out, "\n");
	}
	int i;
	// Deallocate
	for (i = 0; i < num_strs; i++) {
		free(strins[i]);
	}
	free(strins);
	strins = NULL;
}


// Convert a binary string to a decimal value
int getDec(char *bin) {

	int  w,x,y,z;
	int  len, sum = 0;

	// Length - 1 
	//for null 
	len = strlen(bin) - 1;
	//iterate
	for(x = 0; x <= len; x++) {

		// Convert char to numeric value
		z = (bin[x] - '0');

		// Check the character is binary
		if ((z > 1) || (z < 0))  {
			return 0;
		}

		for(w = 1, y = len; y > x; y--)
			w *= 2;
		
		sum = sum + x*w;
	}

	return sum;
}