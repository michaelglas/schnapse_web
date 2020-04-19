/*
 * main.c
 *
 *  Created on: 02.08.2019
 *      Author: michi
 */

#define _GNU_SOURCE
#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include<assert.h>
#include<limits.h>
#include<stddef.h>
#include<unistd.h>
#include<stdint.h>
#include<fcntl.h>

size_t getrandbits(size_t k,int random_stream){
	assert(k<=sizeof(size_t)*CHAR_BIT);
	size_t numbytes = (k + 7) / CHAR_BIT;
	size_t ret;
	read(random_stream, &ret, numbytes);
	return ret >> (numbytes * CHAR_BIT - k);
}

size_t randbelow_incl(size_t n,int random_stream){
	assert(n<=SIZE_MAX);
	size_t k = 0;
	for(size_t nc=n;nc;nc=nc>>1){++k;};
	size_t r;
	do {
		r = getrandbits(k,random_stream);
	} while(r > n);
	return r;
}

int in(const char a,const char* b){
	for(const char* i=b;*i;++i){
		if(*i==a){
			return 1;
		}
	}
	return 0;
}

int in_n(const char a,const char* __restrict b,size_t const length){
	size_t j = 0;
	for(const char* i=b;j<length;++i,++j){
		if(*i==a){
			return 1;
		}
	}
	return 0;
}

int points_per_card[5] = {2,3,4,10,11};
const char* specialchars = "pcs";
const char* numbers = "12345";
const char colors[4] = {'H','S','L','E'};
const char cards[5] = {'U','O','K','Z','A'};

struct card{
	unsigned int color;
	unsigned int card;
};

#define PRINT_ALL(format,...) do {fprintf(streams[0],(format) , ## __VA_ARGS__ );fprintf(streams[1],(format) , ## __VA_ARGS__ );} while(0)
#define PRINT_PLAYER(player,format,...) fprintf(streams[player],(format) , ##__VA_ARGS__ )

#define trumpf stack[0]

#define PRINT_PROMPT PRINT_PLAYER(abs_player,"player %lu",abs_player+1);\
	PRINT_PLAYER(abs_player,";hand ");\
	PRINT_PLAYER(abs_player,"%c%c",colors[hand[abs_player][0].color],cards[hand[abs_player][0].card]);\
	for(unsigned int k=1;k<hand_length;++k){\
		PRINT_PLAYER(abs_player,"|%c%c",colors[hand[abs_player][k].color],cards[hand[abs_player][k].card]);\
	}\
	PRINT_PLAYER(abs_player,";trumpf %c",colors[trumpf.color]);\
	if(!closed){\
		fputc(cards[trumpf.card],streams[abs_player]);\
	}\
	if(rel_player){\
		PRINT_PLAYER(abs_player,";stich %c%c",colors[stich[0]->color],cards[stich[0]->card]);\
	}\
	PRINT_PLAYER(abs_player,"> ");\
	fflush(streams[abs_player])

void shuffle_cards(struct card* restrict x,size_t length,int random_stream){
	for(size_t i=length-1;i>1;--i){
		size_t j = randbelow_incl(i,random_stream);
		struct card temp = x[i];
		x[i] = x[j];
		x[j] = temp;
	}
}

int main(int argc, char **argv) {
	FILE* streams[2];
	if(argc!=3){
		return -1;
	}
	streams[0] = fdopen(atoi(argv[1]), "r+");
	if(errno){
		perror("fdopen");
	}
	streams[1] = fdopen(atoi(argv[2]), "r+");
	if(errno){
		perror("fdopen");
	}
	struct card stack[4*5];
	struct card* end=stack;					   // Initializing end to the start of stack because in the Initialization process we increment end
	for(unsigned int i=0;i<4;++i){     // Initializing stack
		for(unsigned int j=0;j<5;++j){
			*end = (struct card){i,j};
			++end;
		}
	}
	{
		int urandom = open("/dev/urandom",O_RDONLY); // open linux system random generator
		shuffle_cards(stack,4*5,urandom); // shuffle stack
		close(urandom);
	}
	struct card hand[2][5];
	size_t hand_length = 5;
	end -= 5*2;									// by subtracting 10 from end before memcpy end points to the area of memory we want to copy
	memcpy(hand, end, 5*2*sizeof(struct card)); // copy 10 cards from end to hand
	unsigned int points[2]={};					// Initializing points to zeros
	size_t players[2]={0,1};					// Initializing players to identity
	int closed=0;
	int winner=-1;
	while(1){	// infinite loop
		struct card* stich[2];
		for(size_t rel_player=0;rel_player<2;++rel_player){
			size_t abs_player = players[rel_player];
			size_t other_player = !abs_player;
			struct card* selection;
			char* string;
			ssize_t length;
			int special;
			int plus_20_40;
			// parsing:
			int repeat;
			do {
				repeat=0;
				string  = malloc(8);
				length  = 8;
				special = 0;
				PRINT_PROMPT;
				plus_20_40=0;
				length = getline(&string, (size_t*)&length, streams[abs_player]);
				if(length<0){
					if(errno){
						perror("getline");
						return -1;
					}
					return 0;
				}
				while(length!=2||!(in_n(*string,numbers,hand_length)||(!rel_player&&(special=in(*string,specialchars))))){
					free(string);
					string = malloc(8);
					length = 8;
					PRINT_PROMPT;
					length = getline(&string, (size_t*)&length, streams[abs_player]);
					if(length<0){
						if(errno){
							perror("getline");
							return -1;
						}
						return 0;
					}
				}
				if(special){
					if(*string=='p'){
						free(string);
						struct card* pairs[2][2];
						size_t num_pairs=0;
						{
							struct card* potential_pairs[4];
							size_t pp_length=0;
							unsigned int searched_color=(unsigned int)-1;
							for(struct card* i=hand[abs_player];i<hand[abs_player]+hand_length;++i){
								if((i->card==1||i->card==2)&&i->color!=searched_color){
									int pair_found=0;
									for(struct card** j=potential_pairs;j<potential_pairs+pp_length;++j){
										if((*j)->color==i->color){
											pairs[num_pairs][0]=i;
											pairs[num_pairs][1]=*j;
											++num_pairs;
											searched_color=i->color;
											if(j==potential_pairs+pp_length-1){
												--pp_length;
											}
											if(pair_found){
												goto END;
											};
											pair_found=1;
											break;
										}
									}
									if(!pair_found){
										potential_pairs[pp_length] = i;
										++pp_length;
									}
								}
							}
						}
						END:
						if(num_pairs){
							struct card** pair;
							if(num_pairs>1){
								string = malloc(8);
								length = 8;
								PRINT_PLAYER(abs_player,"player %zu;trumpf %c%c;pair1 %c;pair2 %c> ",abs_player+1,colors[trumpf.color],cards[trumpf.card],colors[pairs[0][0]->color],colors[pairs[1][0]->color]);
								fflush(streams[abs_player]);
								length = getline(&string, (size_t*)&length, streams[abs_player]);
								if(length<0){
									if(errno){
										perror("getline");
										return -1;
									}
									return 0;
								}
								while(length!=2||!in(*string,"12")){
									free(string);
									string = malloc(8);
									length = 8;
									PRINT_PLAYER(abs_player,"player %zu;trumpf %c%c;pair1 %c;pair2 %c> ",abs_player+1,colors[trumpf.color],cards[trumpf.card],colors[pairs[0][0]->color],colors[pairs[1][0]->color]);
									fflush(streams[abs_player]);
									length = getline(&string, (size_t*)&length, streams[abs_player]);
									if(length<0){
										if(errno){
											perror("getline");
											return -1;
										}
										return 0;
									}
								}
								pair=pairs[(*string&((1<<2)-1))-1];
								free(string);
							} else {
								pair=pairs[0];
							}
							if((*pair)->color==trumpf.color){
								plus_20_40=2;
							} else {
								plus_20_40=1;
							}
							string = malloc(8);
							length = 8;
							PRINT_PLAYER(abs_player,"player %zu;card1 %c;card2 %c> ",abs_player+1,cards[pair[0]->card],cards[pair[1]->card]);
							fflush(streams[abs_player]);
							length = getline(&string, (size_t*)&length, streams[abs_player]);
							if(length<0){
								if(errno){
									perror("getline");
									return -1;
								}
								return 0;
							}
							while(length!=2||!in(*string,"12")){
								free(string);
								string = malloc(8);
								length = 8;
								PRINT_PLAYER(abs_player,"player %zu;card1 %c;card2 %c> ",abs_player+1,cards[pair[0]->card],cards[pair[1]->card]);
								fflush(streams[abs_player]);
								length = getline(&string, (size_t*)&length, streams[abs_player]);
								if(length<0){
									if(errno){
										perror("getline");
										return -1;
									}
									return 0;
								}
							}
							selection = pair[(*string&((1<<2)-1))-1];
							free(string);
						} else {
							repeat=1;
							PRINT_PLAYER(abs_player,"invalid no pairs\n");
							fflush(streams[abs_player]);
						}
					} else {
						if(end-stack>2){
							switch(*string){
							case'c':
								free(string);
								if(!closed){
									PRINT_PLAYER(other_player,"player %zu closed",abs_player+1);
									fflush(streams[abs_player]);
									closed=1;
								} else {
									PRINT_PLAYER(abs_player,"invalid already closed\n");
									fflush(streams[abs_player]);
								}
								repeat=1;
								break;
							case's':
								free(string);
								for(struct card* i=hand[abs_player];i<hand[abs_player]+hand_length;++i){
									if(i->card==0&&i->color==trumpf.color){
										PRINT_PLAYER(other_player,"player %zu swaped %c\n",abs_player+1,cards[i->card]);
										fflush(streams[abs_player]);
										struct card temp = trumpf;
										trumpf = *i;
										*i = temp;
										goto END1;
									}
								}
								PRINT_PLAYER(abs_player,"invalid no <placeholder>\n");
								fflush(streams[abs_player]);
								END1:
								repeat=1;
								break;
							}
						} else {
							free(string);
						}
					}
				} else {
					selection=hand[abs_player]+((*string&((1<<3)-1))-1);
					free(string);
				}
				if(closed&&rel_player){
					if(selection->color==stich[0]->color){
						if(selection->card<=stich[0]->card){
							for(struct card* i=hand[players[1]];i<hand[players[1]]+hand_length;++i){
								if(i->color==stich[0]->color&&i->card>stich[0]->card){
									repeat=1;
									break;
								}
							}
						}
					} else {
						if(stich[0]->color==trumpf.color){
							if(selection->color!=trumpf.color){
								for(struct card* i=hand[players[1]];i<hand[players[1]]+hand_length;++i){
									if(i->color==trumpf.color){
										repeat=1;
										break;
									}
								}
							}
						} else {
							if(selection->color==trumpf.color){
								for(struct card* i=hand[players[1]];i<hand[players[1]]+hand_length;++i){
									if(i->color==stich[0]->color){
										repeat=1;
										break;
									}
								}
							} else {
								for(struct card* i=hand[players[1]];i<hand[players[1]]+hand_length;++i){
									if(i->color==stich[0]->color||i->color==trumpf.color){
										repeat=1;
										break;
									}
								}
							}
						}
					}
				}
			} while(repeat);
			if(plus_20_40){
				PRINT_PLAYER(other_player,"player %zu showed %c\n",abs_player+1,colors[selection->color]);
				fflush(streams[abs_player]);
				switch(plus_20_40){
				case 1:
					points[abs_player]+=20;
					break;
				case 2:
					points[abs_player]+=40;
					break;
				}
				if(points[abs_player]>=66){
					winner=abs_player;
					goto END_OF_GAME;
				};
			}
			PRINT_PLAYER(other_player,"player %zu played %c%c\n",abs_player+1,colors[selection->color],cards[selection->card]);
			fflush(streams[abs_player]);
			stich[rel_player]=selection;
		}
		if(stich[0]->color==stich[1]->color){
			if(stich[0]->card<stich[1]->card){
				players[0] = players[1];
			}
		} else {
			if(stich[1]->color==trumpf.color){
				players[0] = players[1];
			}
		}
		points[players[0]]+=points_per_card[stich[0]->card]+points_per_card[stich[1]->card];
		players[1] = !players[0];

		if(points[players[0]]>=66){
			winner=players[0];
			goto END_OF_GAME;
		};
		if(!closed){
			if(end-stack){
				*stich[0] = *(--end);
				*stich[1] = *(--end);
			} else {
				closed=1;
				ptrdiff_t offset=(void*)(hand+1)-(void*)stich[0];
				if(offset>0){
					memmove(stich[0], stich[0]+1, offset);
				}
				offset = (void*)(hand+2)-(void*)stich[1];
				if(offset>0){
					memmove(stich[1], stich[1]+1, offset);
				}
				--hand_length;
			}
		} else {
			if(hand_length>1){
				ptrdiff_t offset=(void*)(hand+1)-(void*)stich[0];
				if(offset>0){
					memmove(stich[0], stich[0]+1, offset);
				}
				offset = (void*)(hand+2)-(void*)stich[1];
				if(offset>0){
					memmove(stich[1], stich[1]+1, offset);
				}
				--hand_length;
			} else {
				winner = players[0];
				break;
			}
		}
	}
	END_OF_GAME:
	PRINT_ALL("player %d won\n",winner+1);
}
