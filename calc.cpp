#define _CRT_SECURE_NO_WARNINGS

//状態を構成する変数//
#define I  12 /* イニング */
#define T  2 /* 表裏 */
#define W  4 //w: アウトカウント 0,1,2,3//
#define S  40 //s: 先攻チーム得点で+、後攻チーム得点で-//
#define R1 2 //r1: 1,一塁にランナーがいる 0,ランナーなし//
#define R2 2 //r2: 1,二塁にランナーがいる 0,ランナーなし//
#define R3 2 //r3: 1,三塁にランナーがいる 0,ランナーなし//
#define B1 9 //先攻チームの打順//
#define B2 9 //後攻チームの打順//
#define M  2 /* チーム */

#define UNCHECK -1
#define NAME 9
#define RESULT 9
#define INITIAL_SCORE 20
#define COLD_DIFF 15

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>

using namespace std;

//先攻、後攻チームの各選手のmetricsを格納
double omote[NAME][RESULT], ura[NAME][RESULT];

//チームの各状態での勝率を格納
double table[I][T][W][S][R1][R2][R3][B1][B2][M];

///状態を構成する変数
int i, t, w, s, r1, r2, r3, b1, b2, m;

//先攻、後攻チームの勝率を計算する関数
double calc(int i, int t, int w, int s, int r1, int r2, int r3, int b1, int b2, int m)
{

	b1 = b1 % 9;  //0,1,2,3,4,5,6,7,8
	b2 = b2 % 9;  //0,1,2,3,4,5,6,7,8
	t = t % 2;    //0,1
	double top_batting, top_iw;  
	double bot_batting, bot_iw;  

	if (table[i][t][w][s][r1][r2][r3][b1][b2][m] != UNCHECK) return table[i][t][w][s][r1][r2][r3][b1][b2][m];

    /*吸収状態の定義*/
	else if (i >= 8 && t == 1 && w == 3 && s > INITIAL_SCORE) //先攻リード
	{
		if (m == 0)	return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 1;
		else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 0;
	}
	else if ((i == 8 && t == 0 && w == 3 && s < INITIAL_SCORE) || (i >= 8 && t == 1 && s < INITIAL_SCORE)) //後攻リード
	{
		if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 0;
		else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 1;
	}
	else if (i == 11 && t == 1 && w == 3 && s == INITIAL_SCORE) //延長12回同点で引き分け
	{
		if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 0;
		else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 0;
	}
	else if (s >= INITIAL_SCORE + COLD_DIFF) //先攻コールド勝ち
	{
		if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 1;
		else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 0;
	}
	else if (s <= INITIAL_SCORE - COLD_DIFF) //後攻コールド勝ち
	{
		if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 0;
		else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 1; 
	}             

	// 3アウト
	else if (w == 3){
		if (t == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i    , t + 1, 0, s, 0, 0, 0, b1, b2, m);
		else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i + 1, t    , 0, s, 0, 0, 0, b1, b2, m);
	}          

	// ランナーなし 
	else if (r1 == 0 && r2 == 0 && r3 == 0)
	{
		if (t == 0){
			return table[i][t][w][s][r1][r2][r3][b1][b2][m] = omote[b1][0] * calc(i, t, w + 1, s    , 0, 0, 0, b1 + 1, b2, m)
															+ omote[b1][1] * calc(i, t, w    , s    , 1, 0, 0, b1 + 1, b2, m)
															+ omote[b1][2] * calc(i, t, w    , s    , 0, 1, 0, b1 + 1, b2, m) 
															+ omote[b1][3] * calc(i, t, w    , s    , 0, 0, 1, b1 + 1, b2, m)
															+ omote[b1][4] * calc(i, t, w    , s + 1, 0, 0, 0, b1 + 1, b2, m) 
															+ omote[b1][5] * calc(i, t, w    , s    , 1, 0, 0, b1 + 1, b2, m);
		}
		else
		{
			return table[i][t][w][s][r1][r2][r3][b1][b2][m] = ura[b2][0] * calc(i, t, w + 1, s    , 0, 0, 0, b1, b2 + 1, m)
															+ ura[b2][1] * calc(i, t, w    , s    , 1, 0, 0, b1, b2 + 1, m)
															+ ura[b2][2] * calc(i, t, w    , s    , 0, 1, 0, b1, b2 + 1, m) 
															+ ura[b2][3] * calc(i, t, w    , s    , 0, 0, 1, b1, b2 + 1, m)
															+ ura[b2][4] * calc(i, t, w    , s - 1, 0, 0, 0, b1, b2 + 1, m)
															+ ura[b2][5] * calc(i, t, w    , s    , 1, 0, 0, b1, b2 + 1, m);
		}
	}

	// ランナー:一塁 
	else if (r1 == 1 && r2 == 0 && r3 == 0){
		if (t == 0){
			if (w == 0 || w == 1)
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = (omote[b1][0] - omote[b1][8]) * calc(i, t, w + 1, s    , 1, 0, 0, b1 + 1, b2, m) 
																				 + omote[b1][1] * calc(i, t, w    , s    , 1, 0, 1, b1 + 1, b2, m)
										 										 + omote[b1][2] * calc(i, t, w    , s + 1, 0, 1, 0, b1 + 1, b2, m) 
										 										 + omote[b1][3] * calc(i, t, w    , s + 1, 0, 0, 1, b1 + 1, b2, m)
										 										 + omote[b1][4] * calc(i, t, w    , s + 2, 0, 0, 0, b1 + 1, b2, m) 
										 										 + omote[b1][5] * calc(i, t, w    , s    , 1, 1, 0, b1 + 1, b2, m)
										 										 + omote[b1][8] * calc(i, t, w + 2, s    , 0, 0, 0, b1 + 1, b2, m);
				
			}
			else //w==2//
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = omote[b1][0] * calc(i, t, w + 1, s    , 1, 0, 0, b1 + 1, b2, m) 
																+ omote[b1][1] * calc(i, t, w    , s    , 1, 0, 1, b1 + 1, b2, m)
																+ omote[b1][2] * calc(i, t, w    , s + 1, 0, 1, 0, b1 + 1, b2, m) 
																+ omote[b1][3] * calc(i, t, w    , s + 1, 0, 0, 1, b1 + 1, b2, m)
																+ omote[b1][4] * calc(i, t, w    , s + 2, 0, 0, 0, b1 + 1, b2, m) 
																+ omote[b1][5] * calc(i, t, w    , s    , 1, 1, 0, b1 + 1, b2, m);
			}
		}
		else{ //t=1//			
			if (w == 0 || w == 1)
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = (ura[b2][0] - ura[b2][8]) * calc(i, t, w + 1, s    , 1, 0, 0, b1, b2 + 1, m) 
														    				   + ura[b2][1] * calc(i, t, w    , s    , 1, 0, 1, b1, b2 + 1, m)
																			   + ura[b2][2] * calc(i, t, w    , s - 1, 0, 1, 0, b1, b2 + 1, m) 
																			   + ura[b2][3] * calc(i, t, w    , s - 1, 0, 0, 1, b1, b2 + 1, m)
																		       + ura[b2][4] * calc(i, t, w    , s - 2, 0, 0, 0, b1, b2 + 1, m) 
																			   + ura[b2][5] * calc(i, t, w    , s    , 1, 1, 0, b1, b2 + 1, m)
																			   + ura[b2][8] * calc(i, t, w + 2, s    , 0, 0, 0, b1, b2 + 1, m);
			}
			else //w==2//
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = ura[b2][0] * calc(i, t, w + 1, s    , 1, 0, 0, b1, b2 + 1, m) 
																+ ura[b2][1] * calc(i, t, w    , s    , 1, 0, 1, b1, b2 + 1, m)
																+ ura[b2][2] * calc(i, t, w    , s - 1, 0, 1, 0, b1, b2 + 1, m) 
																+ ura[b2][3] * calc(i, t, w    , s - 1, 0, 0, 1, b1, b2 + 1, m)
																+ ura[b2][4] * calc(i, t, w    , s - 2, 0, 0, 0, b1, b2 + 1, m) 
																+ ura[b2][5] * calc(i, t, w    , s    , 1, 1, 0, b1, b2 + 1, m);

			}
		}
	}

	// ランナー2塁    盗塁なし    凡退、単打、二塁打、三塁打、本塁打、四球の順//
	else if (r1 == 0 && r2 == 1 && r3 == 0){
		if (t == 0){
			top_iw = calc(i, t, w, s, 1, 1, 0, b1 + 1, b2, 0);

			top_batting = omote[b1][0] * calc(i, t, w + 1, s    , 0, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 0, b1 + 1, b2, 0)
						+ omote[b1][2] * calc(i, t, w    , s + 1, 0, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][3] * calc(i, t, w    , s + 1, 0, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][4] * calc(i, t, w    , s + 2, 0, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][5] * calc(i, t, w    , s    , 1, 1, 0, b1 + 1, b2, 0);

			bot_iw = calc(i, t, w, s, 1, 1, 0, b1 + 1, b2, 1);

			bot_batting = omote[b1][0] * calc(i, t, w + 1, s    , 0, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 0, b1 + 1, b2, 1)
						+ omote[b1][2] * calc(i, t, w    , s + 1, 0, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][3] * calc(i, t, w    , s + 1, 0, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][4] * calc(i, t, w    , s + 2, 0, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][5] * calc(i, t, w    , s    , 1, 1, 0, b1 + 1, b2, 1);

			if (bot_iw > bot_batting){ //守備チーム後攻が敬遠を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
			}
			else if (bot_iw < bot_batting){ //守備チーム後攻が投球を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
			}
			else{ //守備チーム後攻にとって敬遠、投球は同じ勝率なので、先攻チームの勝率を下げる戦略を選択
				if (top_iw <= top_batting){
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
				}
				else {
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;	
				}
			}

		}
		else{
			top_iw = calc(i, t, w, s, 1, 1, 0, b1, b2 + 1, 0);

			top_batting = ura[b2][0] * calc(i, t, w + 1, s    , 0, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 0, b1, b2 + 1, 0)
						+ ura[b2][2] * calc(i, t, w    , s - 1, 0, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][3] * calc(i, t, w    , s - 1, 0, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][4] * calc(i, t, w    , s - 2, 0, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][5] * calc(i, t, w    , s    , 1, 1, 0, b1, b2 + 1, 0);

			bot_iw = calc(i, t, w, s, 1, 1, 0, b1, b2 + 1, 1);

			bot_batting = ura[b2][0] * calc(i, t, w + 1, s    , 0, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 0, b1, b2 + 1, 1)
						+ ura[b2][2] * calc(i, t, w    , s - 1, 0, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][3] * calc(i, t, w    , s - 1, 0, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][4] * calc(i, t, w    , s - 2, 0, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][5] * calc(i, t, w    , s    , 1, 1, 0, b1, b2 + 1, 1);

			if (top_iw > top_batting){ //守備チーム先攻が敬遠を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
			}
			else if (top_iw < top_batting){ //守備チーム先攻が投球を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
			}
			else { //守備チーム先攻にとって敬遠、投球は同じ勝率なので、後攻チームの勝率を下げる戦略を選択//
				if (bot_iw <= bot_batting){
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
				} 
				else {
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
				}
			}
		}
	}

	/*ランナー3塁   盗塁なし   凡退、単打、二塁打、三塁打、本塁打、四球の順*/
	else if (r1 == 0 && r2 == 0 && r3 == 1)
	{
		if (t == 0){
			top_iw = calc(i, t, w, s, 1, 0, 1, b1 + 1, b2, 0);

			top_batting = omote[b1][0] * calc(i, t, w + 1, s    , 0, 0, 1, b1 + 1, b2, 0) 
						+ omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 0, b1 + 1, b2, 0)
						+ omote[b1][2] * calc(i, t, w    , s + 1, 0, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][3] * calc(i, t, w    , s + 1, 0, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][4] * calc(i, t, w    , s + 2, 0, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][5] * calc(i, t, w    , s    , 1, 0, 1, b1 + 1, b2, 0);

			bot_iw = calc(i, t, w, s, 1, 0, 1, b1 + 1, b2, 1);

			bot_batting = omote[b1][0] * calc(i, t, w + 1, s    , 0, 0, 1, b1 + 1, b2, 1) 
						+ omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 0, b1 + 1, b2, 1)
						+ omote[b1][2] * calc(i, t, w    , s + 1, 0, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][3] * calc(i, t, w    , s + 1, 0, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][4] * calc(i, t, w    , s + 2, 0, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][5] * calc(i, t, w    , s    , 1, 0, 1, b1 + 1, b2, 1);

			if (bot_iw > bot_batting){ //守備チーム後攻が敬遠を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
			}
			else if (bot_iw < bot_batting){ //守備チーム後攻が投球を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
			}
			else{ //守備チーム後攻にとって敬遠、投球は同じ勝率なので、先攻チームの勝率を下げる戦略を選択
				if (top_iw <= top_batting){
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
				}
				else {
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;	
				}
			}
		}
		else //t==1//
		{
			top_iw = calc(i, t, w, s, 1, 0, 1, b1, b2 + 1, 0);

			top_batting = ura[b2][0] * calc(i, t, w + 1, s    , 0, 0, 1, b1, b2 + 1, 0) 
						+ ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 0, b1, b2 + 1, 0)
						+ ura[b2][2] * calc(i, t, w    , s - 1, 0, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][3] * calc(i, t, w    , s - 1, 0, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][4] * calc(i, t, w    , s - 2, 0, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][5] * calc(i, t, w    , s    , 1, 0, 1, b1, b2 + 1, 0);

			bot_iw = calc(i, t, w, s, 1, 0, 1, b1, b2 + 1, 1);

			bot_batting = ura[b2][0] * calc(i, t, w + 1, s    , 0, 0, 1, b1, b2 + 1, 1) 
						+ ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 0, b1, b2 + 1, 1)
						+ ura[b2][2] * calc(i, t, w    , s - 1, 0, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][3] * calc(i, t, w    , s - 1, 0, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][4] * calc(i, t, w    , s - 2, 0, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][5] * calc(i, t, w    , s    , 1, 0, 1, b1, b2 + 1, 1);

			if (top_iw > top_batting){ //守備チーム先攻が敬遠を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
			}
			else if (top_iw < top_batting){ //守備チーム先攻が投球を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
			}
			else { //守備チーム先攻にとって敬遠、投球は同じ勝率なので、後攻チームの勝率を下げる戦略を選択//
				if (bot_iw <= bot_batting){
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
				} 
				else {
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
				}
			}
		}
	}

	// ランナー1,2塁    　盗塁なし   凡退、単打、二塁打、三塁打、本塁打、四球の順//
	else if (r1 == 1 && r2 == 1 && r3 == 0)
	{
		if (t == 0){
			if (w == 0 || w == 1)
			{		
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = (omote[b1][0] - omote[b1][8]) * calc(i, t, w + 1, s    , 1, 1, 0, b1 + 1, b2, m) 
										   										 + omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 1, b1 + 1, b2, m)
										   										 + omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, m) 
										   										 + omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, m)
										   										 + omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, m) 
										   										 + omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, m)
										   										 + omote[b1][8] * calc(i, t, w + 2, s    , 0, 0, 1, b1 + 1, b2, m);

			}
			else{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = omote[b1][0] * calc(i, t, w + 1, s    , 1, 1, 0, b1 + 1, b2, m) 
																+ omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 1, b1 + 1, b2, m)
																+ omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, m) 
																+ omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, m)
																+ omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, m) 
																+ omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, m);
			}
		}
		else //t==1//
		{
			if (w == 0 || w == 1)
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s    , 1, 1, 0, b1, b2 + 1, m) 
										 									 + ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 1, b1, b2 + 1, m)
										 									 + ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, m) 
										 									 + ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, m)
										 									 + ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, m) 
										 									 + ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, m)
										 									 + ura[b2][8] * calc(i, t, w + 2, s    , 0, 0, 1, b1, b2 + 1, m);
			}
			else //w==2//
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = ura[b2][0] * calc(i, t, w + 1, s    , 1, 1, 0, b1, b2 + 1, m) 
																+ ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 1, b1, b2 + 1, m)
																+ ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, m) 
																+ ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, m)
																+ ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, m) 
																+ ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, m);
			}
		}
	}

	// ランナー2,3塁    盗塁なし   凡退、単打、二塁打、三塁打、本塁打、四球の順 //
	else if (r1 == 0 && r2 == 1 && r3 == 1)
	{
		if (t == 0){
			top_iw = calc(i, t, w, s, 1, 1, 1, b1 + 1, b2, 0);

			top_batting = omote[b1][0] * calc(i, t, w + 1, s    , 0, 1, 1, b1 + 1, b2, 0) 
						+ omote[b1][1] * calc(i, t, w    , s + 2, 1, 0, 0, b1 + 1, b2, 0)
						+ omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, 0);

			bot_iw = calc(i, t, w, s, 1, 1, 1, b1 + 1, b2, 1);

			bot_batting = omote[b1][0] * calc(i, t, w + 1, s    , 0, 1, 1, b1 + 1, b2, 1) 
						+ omote[b1][1] * calc(i, t, w    , s + 2, 1, 0, 0, b1 + 1, b2, 1)
						+ omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, 1);
			
			if(bot_iw > bot_batting){ //守備チーム後攻が敬遠を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
			}
			else if (bot_iw < bot_batting){ //守備チーム後攻が投球を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
			}
			else{ //守備チームにとっては敬遠、投球どちらでも同じ勝率なので、先攻の勝率を下げる戦略を選択
				if (top_iw <= top_batting){ //守備チームは敬遠を選択//
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
				}
				else{ //守備チームは投球を選択//
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
				}
			}	
		}
		else //t==1//
		{
			top_iw = calc(i, t, w, s, 1, 1, 1, b1, b2 + 1, 0);

			top_batting = ura[b2][0] * calc(i, t, w + 1, s    , 0, 1, 1, b1, b2 + 1, 0) 
						+ ura[b2][1] * calc(i, t, w    , s - 2, 1, 0, 0, b1, b2 + 1, 0)
						+ ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, 0);

			bot_iw = calc(i, t, w, s, 1, 1, 1, b1, b2 + 1, 1);
								
			bot_batting = ura[b2][0] * calc(i, t, w + 1, s    , 0, 1, 1, b1, b2 + 1, 1) 
						+ ura[b2][1] * calc(i, t, w    , s - 2, 1, 0, 0, b1, b2 + 1, 1)
						+ ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, 1);
								
			if(top_iw > top_batting){ //守備チーム先攻が敬遠を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
				else  		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
			}
			else if (top_iw < top_batting){ //守備チーム先攻が投球を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
			}
			else{ //守備チームにとっては敬遠、投球どちらでも同じ勝率なので、後攻の勝率を下げる戦略を選択//
				if (bot_iw <= bot_batting){ //守備チームは敬遠を選択//
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;			
				}
				else{ //守備チームは投球を選択//
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
				}
			}
		}
	}

	/*ランナー1,3塁  凡退、単打、二塁打、三塁打、本塁打、四球の順*/
	else if (r1 == 1 && r2 == 0 && r3 == 1)
	{
		if (t == 0){
			top_iw = calc(i, t, w, s, 1, 1, 1, b1 + 1, b2, 0);
			bot_iw = calc(i, t, w, s, 1, 1, 1, b1 + 1, b2, 1);

			if (w == 0)
			{
				top_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s    , 1, 0, 1, b1 + 1, b2, 0) 
										   + omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 1, b1 + 1, b2, 0)
										   + omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, 0) 
										   + omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, 0)
										   + omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, 0) 
										   + omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, 0)
										   + omote[b1][8] * calc(i, t, w + 2, s + 1, 0, 0, 0, b1 + 1, b2, 0);

				bot_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s    , 1, 0, 1, b1 + 1, b2, 1) 
										   + omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 1, b1 + 1, b2, 1)
										   + omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, 1) 
										   + omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, 1)
										   + omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, 1) 
										   + omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, 1)
										   + omote[b1][8] * calc(i, t, w + 2, s + 1, 0, 0, 0, b1 + 1, b2, 1);
			}
			else if (w == 1)
			{
				top_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s    , 1, 0, 1, b1 + 1, b2, 0) 
										   + omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 1, b1 + 1, b2, 0)
										   + omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, 0) 
										   + omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, 0)
										   + omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, 0) 
										   + omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, 0)
										   + omote[b1][8] * calc(i, t, w + 2, s    , 0, 0, 0, b1 + 1, b2, 0);

				bot_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s    , 1, 0, 1, b1 + 1, b2, 1) 
										   + omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 1, b1 + 1, b2, 1)
										   + omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, 1) 
										   + omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, 1)
										   + omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, 1) 
										   + omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, 1)
										   + omote[b1][8] * calc(i, t, w + 2, s    , 0, 0, 0, b1 + 1, b2, 1);
			}
			else //w==2//
			{
				top_batting = omote[b1][0] * calc(i, t, w + 1, s    , 1, 0, 1, b1 + 1, b2, 0) 
							+ omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 1, b1 + 1, b2, 0)
							+ omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, 0) 
							+ omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, 0)
							+ omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, 0) 
							+ omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, 0);

				bot_batting = omote[b1][0] * calc(i, t, w + 1, s    , 1, 0, 1, b1 + 1, b2, 1) 
							+ omote[b1][1] * calc(i, t, w    , s + 1, 1, 0, 1, b1 + 1, b2, 1)
							+ omote[b1][2] * calc(i, t, w    , s + 2, 0, 1, 0, b1 + 1, b2, 1) 
							+ omote[b1][3] * calc(i, t, w    , s + 2, 0, 0, 1, b1 + 1, b2, 1)
							+ omote[b1][4] * calc(i, t, w    , s + 3, 0, 0, 0, b1 + 1, b2, 1) 
							+ omote[b1][5] * calc(i, t, w    , s    , 1, 1, 1, b1 + 1, b2, 1);
			}

			if(bot_iw > bot_batting){ //守備チーム後攻が敬遠を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
			}
			else if (bot_iw < bot_batting){ //守備チーム後攻が投球を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
			}
			else{ //守備チーム後攻にとって投球、敬遠は変わりがないので、先攻チームの勝率を下げる戦略を選ぶ
				if(top_iw <= top_batting){ //敬遠を選択//
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
				}
				else{ //投球を選択//
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;	
				}
			} 
		}
		else //t==1//
		{
			top_iw = calc(i, t, w, s, 1, 1, 1, b1, b2 + 1, 0);
			bot_iw = calc(i, t, w, s, 1, 1, 1, b1, b2 + 1, 1);

			if (w == 0){
				top_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s    , 1, 0, 1, b1, b2 + 1, 0) 
										 + ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 1, b1, b2 + 1, 0)
										 + ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, 0) 
										 + ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, 0)
										 + ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, 0) 
										 + ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, 0)
										 + ura[b2][8] * calc(i, t, w + 2, s - 1, 0, 0, 0, b1, b2 + 1, 0);

				bot_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s    , 1, 0, 1, b1, b2 + 1, 1) 
										 + ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 1, b1, b2 + 1, 1)
										 + ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, 1) 
										 + ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, 1)
										 + ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, 1) 
										 + ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, 1)
										 + ura[b2][8] * calc(i, t, w + 2, s - 1, 0, 0, 0, b1, b2 + 1, 1);
			}
			else if (w == 1){
				top_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s    , 1, 0, 1, b1, b2 + 1, 0) 
										 + ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 1, b1, b2 + 1, 0)
										 + ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, 0) 
										 + ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, 0)
										 + ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, 0) 
										 + ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, 0)
										 + ura[b2][8] * calc(i, t, w + 2, s    , 0, 0, 0, b1, b2 + 1, 0);

				bot_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s    , 1, 0, 1, b1, b2 + 1, 1) 
										 + ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 1, b1, b2 + 1, 1)
										 + ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, 1) 
										 + ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, 1)
										 + ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, 1) 
										 + ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, 1)
										 + ura[b2][8] * calc(i, t, w + 2, s    , 0, 0, 0, b1, b2 + 1, 1);
			}
			else //w==2//
			{
				top_batting = ura[b2][0] * calc(i, t, w + 1, s    , 1, 0, 1, b1, b2 + 1, 0) 
							+ ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 1, b1, b2 + 1, 0)
							+ ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, 0) 
							+ ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, 0)
							+ ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, 0) 
							+ ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, 0);

				bot_batting = ura[b2][0] * calc(i, t, w + 1, s    , 1, 0, 1, b1, b2 + 1, 1) 
							+ ura[b2][1] * calc(i, t, w    , s - 1, 1, 0, 1, b1, b2 + 1, 1)
							+ ura[b2][2] * calc(i, t, w    , s - 2, 0, 1, 0, b1, b2 + 1, 1) 
							+ ura[b2][3] * calc(i, t, w    , s - 2, 0, 0, 1, b1, b2 + 1, 1)
							+ ura[b2][4] * calc(i, t, w    , s - 3, 0, 0, 0, b1, b2 + 1, 1) 
							+ ura[b2][5] * calc(i, t, w    , s    , 1, 1, 1, b1, b2 + 1, 1);
			}

			if(top_iw > top_batting){ //守備チーム先攻が敬遠を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
			}
			else if (top_iw < top_batting){ //守備チーム先攻が投球を選択//
				if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
				else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;
			}
			else{ //守備チーム先攻にとって投球、敬遠は変わりがないので、後攻チームの勝率を下げる戦略を選ぶ//
				if(bot_iw <= bot_batting){ //敬遠を選択//
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_iw;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_iw;
				}
				else{ //投球を選択//
					if (m == 0) return table[i][t][w][s][r1][r2][r3][b1][b2][m] = top_batting;
					else 		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = bot_batting;	
				}
			} 
		}
	}

	/*ランナー満塁   盗塁なし 敬遠無し 凡退、単打、二塁打、三塁打、本塁打、四球の順*/
	else if (r1 == 1 && r2 == 1 && r3 == 1)
	{
		if (t == 0){
			if (w == 0)
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = (omote[b1][0] - omote[b1][8]) * calc(i, t, w + 1, s    , 1, 1, 1, b1 + 1, b2, m) 
																		 		 + omote[b1][1] * calc(i, t, w    , s + 2, 1, 0, 1, b1 + 1, b2, m)
																				 + omote[b1][2] * calc(i, t, w    , s + 3, 0, 1, 0, b1 + 1, b2, m)
																		    	 + omote[b1][3] * calc(i, t, w    , s + 3, 0, 0, 1, b1 + 1, b2, m)
																		    	 + omote[b1][4] * calc(i, t, w    , s + 4, 0, 0, 0, b1 + 1, b2, m) 
																			   	 + omote[b1][5] * calc(i, t, w    , s + 1, 1, 1, 1, b1 + 1, b2, m)
																				 + omote[b1][8] * calc(i, t, w + 2, s + 1, 0, 1, 1, b1 + 1, b2, m);
			}
			else if (w == 1)
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = (omote[b1][0] - omote[b1][8]) * calc(i, t, w + 1, s    , 1, 1, 1, b1 + 1, b2, m) 
																		 		 + omote[b1][1] * calc(i, t, w    , s + 2, 1, 0, 1, b1 + 1, b2, m)
																				 + omote[b1][2] * calc(i, t, w    , s + 3, 0, 1, 0, b1 + 1, b2, m)
																		    	 + omote[b1][3] * calc(i, t, w    , s + 3, 0, 0, 1, b1 + 1, b2, m)
																		    	 + omote[b1][4] * calc(i, t, w    , s + 4, 0, 0, 0, b1 + 1, b2, m) 
																			   	 + omote[b1][5] * calc(i, t, w    , s + 1, 1, 1, 1, b1 + 1, b2, m)
																				 + omote[b1][8] * calc(i, t, w + 2, s    , 0, 1, 1, b1 + 1, b2, m);
			}
			else //w==2//
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = omote[b1][0] * calc(i, t, w + 1, s    , 1, 1, 1, b1 + 1, b2, m) 
																+ omote[b1][1] * calc(i, t, w    , s + 2, 1, 0, 1, b1 + 1, b2, m)
																+ omote[b1][2] * calc(i, t, w    , s + 3, 0, 1, 0, b1 + 1, b2, m) 
																+ omote[b1][3] * calc(i, t, w    , s + 3, 0, 0, 1, b1 + 1, b2, m)
																+ omote[b1][4] * calc(i, t, w    , s + 4, 0, 0, 0, b1 + 1, b2, m) 
																+ omote[b1][5] * calc(i, t, w    , s + 1, 1, 1, 1, b1 + 1, b2, m);
			}
		}
		else
		{
			if (w == 0)
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = (ura[b2][0] - ura[b2][8]) * calc(i, t, w + 1, s    , 1, 1, 1, b1, b2 + 1, m) 
																			   + ura[b2][1] * calc(i, t, w    , s - 2, 1, 0, 1, b1, b2 + 1, m)
																			   + ura[b2][2] * calc(i, t, w    , s - 3, 0, 1, 0, b1, b2 + 1, m) 
																			   + ura[b2][3] * calc(i, t, w    , s - 3, 0, 0, 1, b1, b2 + 1, m)
																			   + ura[b2][4] * calc(i, t, w    , s - 4, 0, 0, 0, b1, b2 + 1, m) 
																			   + ura[b2][5] * calc(i, t, w    , s - 1, 1, 1, 1, b1, b2 + 1, m)
																			   + ura[b2][8] * calc(i, t, w + 2, s - 1, 0, 1, 1, b1, b2 + 1, m);
			}
			else if (w == 1)
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = (ura[b2][0] - ura[b2][8]) * calc(i, t, w + 1, s    , 1, 1, 1, b1, b2 + 1, m) 
																			   + ura[b2][1] * calc(i, t, w    , s - 2, 1, 0, 1, b1, b2 + 1, m)
																			   + ura[b2][2] * calc(i, t, w    , s - 3, 0, 1, 0, b1, b2 + 1, m) 
																			   + ura[b2][3] * calc(i, t, w    , s - 3, 0, 0, 1, b1, b2 + 1, m)
																			   + ura[b2][4] * calc(i, t, w    , s - 4, 0, 0, 0, b1, b2 + 1, m) 
																			   + ura[b2][5] * calc(i, t, w    , s - 1, 1, 1, 1, b1, b2 + 1, m)
																			   + ura[b2][8] * calc(i, t, w + 2, s    , 0, 1, 1, b1, b2 + 1, m);
			}
			else
			{
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = ura[b2][0] * calc(i, t, w + 1, s    , 1, 1, 1, b1, b2 + 1, m) 
																+ ura[b2][1] * calc(i, t, w    , s - 2, 1, 0, 1, b1, b2 + 1, m)
																+ ura[b2][2] * calc(i, t, w    , s - 3, 0, 1, 0, b1, b2 + 1, m) 
																+ ura[b2][3] * calc(i, t, w    , s - 3, 0, 0, 1, b1, b2 + 1, m)
																+ ura[b2][4] * calc(i, t, w    , s - 4, 0, 0, 0, b1, b2 + 1, m) 
																+ ura[b2][5] * calc(i, t, w    , s - 1, 1, 1, 1, b1, b2 + 1, m);
			}
		}
	}

	else {
		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 0;
	}

}

int main(void)
{
	double top_wp, bot_wp;

	/*先攻チームのデータ読み取り*/
	FILE *ff = fopen("./data/top.txt", "r");

	// 最初の1行（ヘッダー行）を丸ごと読み飛ばす
	char dummy[1024];
	fgets(dummy, sizeof(dummy), ff);

	for (int row = 0; row < NAME; row++){
		// 一列目は選手名なので読み飛ばす
		fscanf(ff, "%*s");
		for (int col = 0; col < RESULT; col++)
		{
			fscanf(ff, "%lf", &omote[row][col]);
		}
	}    

	/*後攻チームのデータ読み取り*/
	FILE *fg = fopen("./data/bottom.txt", "r");

	// 最初の1行（ヘッダー行）を丸ごと読み飛ばす
	fgets(dummy, sizeof(dummy), fg);

	for (int row = 0; row < NAME; row++){
		// 一列目は選手名なので読み飛ばす
		fscanf(fg, "%*s");
		for (int col = 0; col < RESULT; col++)
		{
			fscanf(fg, "%lf", &ura[row][col]);
		}
	}	

	for (i = 0; i < I; i++){
		for (t = 0; t < T; t++){
			for (w = 0; w < W; w++){
				for (s = 0; s < S; s++){
					for (r1 = 0; r1 < R1; r1++){
						for (r2 = 0; r2 < R2; r2++){
							for (r3 = 0; r3 < R3; r3++){
								for (b1 = 0; b1 < B1; b1++){
									for (b2 = 0; b2 < B2; b2++){
										for (m = 0; m < M; m++){
											table[i][t][w][s][r1][r2][r3][b1][b2][m] = UNCHECK;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	i = 0;
	t = 0;
	w = 0;
	s = 20;
	r1 = 0;
	r2 = 0;
	r3 = 0;
	b1 = 0;
	b2 = 0;

	top_wp = calc(i, t, w, s, r1, r2, r3, b1, b2, 0);
	bot_wp = calc(i, t, w, s, r1, r2, r3, b1, b2, 1);

	printf("先攻の勝率: %lf\n", top_wp);
	printf("後攻の勝率: %lf\n", bot_wp);

	return 0;
}