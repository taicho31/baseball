#define _CRT_SECURE_NO_WARNINGS

//状態を構成する変数//
#define I  12 /* イニング */
#define T  2 /* チーム */
#define W  4 //w: アウトカウント 0,1,2//
#define S  40 //s: 先攻チーム得点で+、後攻チーム得点で-//
#define R1 10 //r1: 0から8番が打順、9は一塁にランナーがいないとき//
#define R2 2 //r2: 1,二塁にランナーがいる 0,ランナーなし//
#define R3 2 //r3: 1,三塁にランナーがいる 0,ランナーなし//
#define B1 9 //先攻チームの打順//
#define B2 9 //後攻チームの打順//
#define M  2 /* チーム */

#define UNCHECK -1
#define NAME 9
#define RESULT 9

#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
#include <fstream>

using namespace std;

//先攻、後攻チームの各選手のmetricsを格納
double omote[NAME][RESULT], ura[NAME][RESULT];

//チームの各状態での勝率を格納
double table[I][T][W][S][R1][R2][R3][B1][B2][M];

//各状態での最適戦略を格納
//H:Hitting, S: Stole Base, B: Bant, N: None, W: Intentional walk, P: pitching
char stable[I][T][W][S][R1][R2][R3][B1][B2][M];

///状態を構成する変数
int i, t, w, s, r1, r2, r3, b1, b2, m;

//Kira,Inakawaの再現
//2,3塁での併殺打はなし
//犠飛なし
//敬遠は満塁以外の得点圏でのみ利用可能

//先攻、後攻チームの勝率を計算する関数
double calc(int i, int t, int w, int s, int r1, int r2, int r3, int b1, int b2, int m)
{

	b1 = b1 % 9;  //0,1,2,3,4,5,6,7,8
	b2 = b2 % 9;  //0,1,2,3,4,5,6,7,8
	t = t % 2;    //0,1
	//a:打撃併殺あり,b:盗塁,c:犠打,d:打撃併殺なし,e:敬遠
	int initial_score = 20;
	double top_batting, top_steal, top_bant, top_iw;  
	double bot_batting, bot_steal, bot_bant, bot_iw;  

	if (table[i][t][w][s][r1][r2][r3][b1][b2][m] != UNCHECK)
		return table[i][t][w][s][r1][r2][r3][b1][b2][m];

    /*吸収状態の定義*/
	else if (i >= 8 && t == 1 && w == 3 && s > initial_score) //9回裏以降終了時先攻リード
	{
		stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
		stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
		table[i][t][w][s][r1][r2][r3][b1][b2][0] = 1;
		table[i][t][w][s][r1][r2][r3][b1][b2][1] = 0;
		return table[i][t][w][s][r1][r2][r3][b1][b2][m];

	}
	else if ((i == 8 && t == 0 && w == 3 && s < initial_score) || (i >= 8 && t == 1 && s < initial_score)) //後攻リード
	{
		stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
		stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
		table[i][t][w][s][r1][r2][r3][b1][b2][0] = 0;
		table[i][t][w][s][r1][r2][r3][b1][b2][1] = 1;
		return table[i][t][w][s][r1][r2][r3][b1][b2][m];
	}
	else if (i == 11 && t == 1 && w == 3 && s == initial_score) //延長12回同点で引き分け
	{
		stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
		stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
		table[i][t][w][s][r1][r2][r3][b1][b2][0] = 0;
		table[i][t][w][s][r1][r2][r3][b1][b2][1] = 0;
		return table[i][t][w][s][r1][r2][r3][b1][b2][m];
	}
	else if (s >= initial_score + 15) //15点差以上で先攻コールド勝ち
	{
		stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
		stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
		table[i][t][w][s][r1][r2][r3][b1][b2][0] = 1;
		table[i][t][w][s][r1][r2][r3][b1][b2][1] = 0;
		return table[i][t][w][s][r1][r2][r3][b1][b2][m];
	}
	else if (s <= initial_score - 15) //15点差以上で後攻コールド負け
	{
		stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
		stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
		table[i][t][w][s][r1][r2][r3][b1][b2][0] = 0;
		table[i][t][w][s][r1][r2][r3][b1][b2][1] = 1; 
		return table[i][t][w][s][r1][r2][r3][b1][b2][m];
	}                       

	// ランナーなし   盗塁、犠打なし  凡退、単打、二塁打、三塁打、本塁打、四球の順
	else if (r1 == 9 && r2 == 0 && r3 == 0)
	{
		if (t == 0){
			if (w == 3)
			{
				stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
				stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
				table[i][t][w][s][r1][r2][r3][b1][b2][0] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, 0);
				table[i][t][w][s][r1][r2][r3][b1][b2][1] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, 1);
				return table[i][t][w][s][r1][r2][r3][b1][b2][m];
			}
			else //w=0,1,2
			{
				//投球時
				bot_batting = omote[b1][0] * calc(i, t, w + 1, s, 9, 0, 0, b1 + 1, b2, 1)
					+ omote[b1][1] * calc(i, t, w, s, b1, 0, 0, b1 + 1, b2, 1)
					+ omote[b1][2] * calc(i, t, w, s, 9, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][3] * calc(i, t, w, s, 9, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][4] * calc(i, t, w, s + 1, 9, 0, 0, b1 + 1, b2, 1) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 0, 0, b1 + 1, b2, 1);

				top_batting = omote[b1][0] * calc(i, t, w + 1, s, 9, 0, 0, b1 + 1, b2, 0)
					+ omote[b1][1] * calc(i, t, w, s, b1, 0, 0, b1 + 1, b2, 0)
					+ omote[b1][2] * calc(i, t, w, s, 9, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][3] * calc(i, t, w, s, 9, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][4] * calc(i, t, w, s + 1, 9, 0, 0, b1 + 1, b2, 0) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 0, 0, b1 + 1, b2, 0);

				stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
				table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
				stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
				table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
				return table[i][t][w][s][r1][r2][r3][b1][b2][m];
			}
		}
		else
		{
			if (w == 3)
			{
				stable[i][t][w][s][r1][r2][r3][b1][b2][0]= 'N';		
				stable[i][t][w][s][r1][r2][r3][b1][b2][1]= 'N';
				table[i][t][w][s][r1][r2][r3][b1][b2][0] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, 0);
				table[i][t][w][s][r1][r2][r3][b1][b2][1] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, 1);
				return table[i][t][w][s][r1][r2][r3][b1][b2][m];
			}
			else //w==0,1,2//
			{
				//投球時//
				top_batting = ura[b1][0] * calc(i, t, w + 1, s, 9, 0, 0, b1, b2 + 1, 0)
					+ ura[b1][1] * calc(i, t, w, s, b1, 0, 0, b1, b2 + 1, 0)
					+ ura[b1][2] * calc(i, t, w, s, 9, 1, 0, b1, b2 + 1, 0) 
					+ ura[b1][3] * calc(i, t, w, s, 9, 0, 1, b1, b2 + 1, 0)
					+ ura[b1][4] * calc(i, t, w, s + 1, 9, 0, 0, b1, b2 + 1, 0) 
					+ ura[b1][5] * calc(i, t, w, s, b1, 0, 0, b1, b2 + 1, 0);

				bot_batting = ura[b1][0] * calc(i, t, w + 1, s, 9, 0, 0, b1, b2 + 1, 1)
				+ ura[b1][1] * calc(i, t, w, s, b1, 0, 0, b1, b2 + 1, 1)
				+ ura[b1][2] * calc(i, t, w, s, 9, 1, 0, b1, b2 + 1, 1) 
				+ ura[b1][3] * calc(i, t, w, s, 9, 0, 1, b1, b2 + 1, 1)
				+ ura[b1][4] * calc(i, t, w, s + 1, 9, 0, 0, b1, b2 + 1, 1) 
				+ ura[b1][5] * calc(i, t, w, s, b1, 0, 0, b1, b2 + 1, 1);

				stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
				table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
				stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
				table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;

				return table[i][t][w][s][r1][r2][r3][b1][b2][m];
			}
		}
	}
	/* ランナー:一塁   凡退、単打、二塁打、三塁打、本塁打、四球、併殺打の順*/
	else if (r1 != 9 && r2 == 0 && r3 == 0){
		if (t == 0){
			if (w == 3)
			{
				stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
				stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
				table[i][t][w][s][r1][r2][r3][b1][b2][0] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, 0);
				table[i][t][w][s][r1][r2][r3][b1][b2][1] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, 1);
				return table[i][t][w][s][r1][r2][r3][b1][b2][m];
			}
			else
			{
				//投球時//
				/*bot_batting = (omote[b1][0] - omote[b1][8]) * calc(i, t, w + 1, s, r1, 0, 0, b1 + 1, b2, 1) 
					+ omote[b1][1] * calc(i, t, w, s, b1, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][2] * calc(i, t, w, s + 1, 9, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][3] * calc(i, t, w, s + 1, 9, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][4] * calc(i, t, w, s + 2, 9, 0, 0, b1 + 1, b2, 1) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 0, b1 + 1, b2, 1)
					+ omote[b1][8] * calc(i, t, w + 2, s, 9, 0, 0, b1 + 1, b2, 1);*/

				bot_steal = omote[b1][6] * calc(i, t, w, s, 9, 1, 0, b1, b2, 1) 
					+ (1.0 - omote[b1][6])*calc(i, t, w + 1, s, 9, 0, 0, b1, b2, 1);

				bot_bant = omote[b1][7] * calc(i, t, w + 1, s, 9, 1, 0, b1 + 1, b2, 1) 
					+ (1.0 - omote[b1][7])*calc(i, t, w + 1, s, b1, 0, 0, b1 + 1, b2, 1);

				/*top_batting = (omote[b1][0] - omote[b1][8]) * calc(i, t, w + 1, s, r1, 0, 0, b1 + 1, b2, 0) 
					+ omote[b1][1] * calc(i, t, w, s, b1, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][2] * calc(i, t, w, s + 1, 9, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][3] * calc(i, t, w, s + 1, 9, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][4] * calc(i, t, w, s + 2, 9, 0, 0, b1 + 1, b2, 0) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 0, b1 + 1, b2, 0)
					+ omote[b1][8] * calc(i, t, w + 2, s, 9, 0, 0, b1 + 1, b2, 0);*/

				top_steal = omote[b1][6] * calc(i, t, w, s, 9, 1, 0, b1, b2, 0) 
					+ (1.0 - omote[b1][6])*calc(i, t, w + 1, s, 9, 0, 0, b1, b2, 0);

				top_bant = omote[b1][7] * calc(i, t, w + 1, s, 9, 1, 0, b1 + 1, b2, 0) 
					+ (1.0 - omote[b1][7])*calc(i, t, w + 1, s, b1, 0, 0, b1 + 1, b2, 0);

				bot_batting = omote[b1][0]  * calc(i, t, w + 1, s, r1, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][1] * calc(i, t, w, s, b1, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][2] * calc(i, t, w, s + 1, 9, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][3] * calc(i, t, w, s + 1, 9, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][4] * calc(i, t, w, s + 2, 9, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][5] * calc(i, t, w, s, b1, 1, 0, b1 + 1, b2, 1);

				top_batting = omote[b1][0]  * calc(i, t, w + 1, s, r1, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][1] * calc(i, t, w, s, b1, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][2] * calc(i, t, w, s + 1, 9, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][3] * calc(i, t, w, s + 1, 9, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][4] * calc(i, t, w, s + 2, 9, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][5] * calc(i, t, w, s, b1, 1, 0, b1 + 1, b2, 0);

				if (w == 0 || w == 1)
				{
					//守備チーム後攻が投球を選択する//
					if (top_batting == max({top_batting, top_steal, top_bant})){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_steal == max({top_batting, top_steal, top_bant})){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'S';
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else {
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
				}
				else //w==2//
				{
					/*ここに置き換え*/

					if (top_steal > top_batting){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'S';
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else {
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
				}
			}
		}
		else{ //t=1//
			if (w == 3)
			{
				stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
				stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
				table[i][t][w][s][r1][r2][r3][b1][b2][0] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, 0);
				table[i][t][w][s][r1][r2][r3][b1][b2][1] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, 1);
				return table[i][t][w][s][r1][r2][r3][b1][b2][m];
			}
			else
			{
				//投球時//
				top_steal = ura[b2][6] * calc(i, t, w, s, 9, 1, 0, b1, b2, 0) 
					+ (1.0 - ura[b2][6]) * calc(i, t, w + 1, s, 9, 0, 0, b1, b2, 0);

				/*top_batting = (ura[b2][0] - ura[b2][8]) * calc(i, t, w + 1, s, r1, 0, 0, b1, b2 + 1, 0) 
					+ ura[b2][1] * calc(i, t, w, s, b2, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][2] * calc(i, t, w, s - 1, 9, 1, 0, b1, b2 + 1, 0) 
					+ ura[b2][3] * calc(i, t, w, s - 1, 9, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][4] * calc(i, t, w, s - 2, 9, 0, 0, b1, b2 + 1, 0) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 0, b1, b2 + 1, 0)
					+ ura[b2][8] * calc(i, t, w + 2, s, 9, 0, 0, b1, b2 + 1, 0);*/

				top_bant = ura[b2][7] * calc(i, t, w + 1, s, 9, 1, 0, b1, b2 + 1, 0) 
					+ (1.0 - ura[b2][7]) *calc(i, t, w + 1, s, b2, 0, 0, b1, b2 + 1, 0);

				/*bot_batting = (ura[b2][0] - ura[b2][8]) * calc(i, t, w + 1, s, r1, 0, 0, b1, b2 + 1, 1) 
					+ ura[b2][1] * calc(i, t, w, s, b2, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][2] * calc(i, t, w, s - 1, 9, 1, 0, b1, b2 + 1, 1) 
					+ ura[b2][3] * calc(i, t, w, s - 1, 9, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][4] * calc(i, t, w, s - 2, 9, 0, 0, b1, b2 + 1, 1) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 0, b1, b2 + 1, 1)
					+ ura[b2][8] * calc(i, t, w + 2, s, 9, 0, 0, b1, b2 + 1, 1);*/

				bot_steal = ura[b2][6] * calc(i, t, w, s, 9, 1, 0, b1, b2, 1) 
					+ (1.0 - ura[b2][6]) * calc(i, t, w + 1, s, 9, 0, 0, b1, b2, 1);

				bot_bant = ura[b2][7] * calc(i, t, w + 1, s, 9, 1, 0, b1, b2 + 1, 1) 
					+ (1.0 - ura[b2][7]) *calc(i, t, w + 1, s, b2, 0, 0, b1, b2 + 1, 1);

				top_batting = ura[b2][0] * calc(i, t, w + 1, s, r1, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][1] * calc(i, t, w, s, b2, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][2] * calc(i, t, w, s - 1, 9, 1, 0, b1, b2 + 1, 0)  
						+ ura[b2][3] * calc(i, t, w, s - 1, 9, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][4] * calc(i, t, w, s - 2, 9, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][5] * calc(i, t, w, s, b2, 1, 0, b1, b2 + 1, 0);

				bot_batting = ura[b2][0] * calc(i, t, w + 1, s, r1, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][1] * calc(i, t, w, s, b2, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][2] * calc(i, t, w, s - 1, 9, 1, 0, b1, b2 + 1, 1)  
						+ ura[b2][3] * calc(i, t, w, s - 1, 9, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][4] * calc(i, t, w, s - 2, 9, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][5] * calc(i, t, w, s, b2, 1, 0, b1, b2 + 1, 1);
			
				if (w == 0 || w == 1)
				{
					//守備チーム先攻が投球を選択する//
					if (bot_batting == max({bot_batting, bot_steal, bot_bant})){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];						
					}
					else if (bot_steal == max({bot_batting, bot_steal, bot_bant})){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'S';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];	
					}
					else {
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
				}

				else //w==2//
				{
					/*ここに置き換え*/

					if (top_steal > top_batting){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'S';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else {
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}	
				}
			}
		}
	}
	// ランナー2塁    盗塁なし    凡退、単打、二塁打、三塁打、本塁打、四球の順//
	else if (r1 == 9 && r2 == 1 && r3 == 0){
		if (t == 0){
			if (w == 3){
				stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
				stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
				table[i][t][w][s][r1][r2][r3][b1][b2][0] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, 0);
				table[i][t][w][s][r1][r2][r3][b1][b2][1] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, 1);
				return table[i][t][w][s][r1][r2][r3][b1][b2][m];
			}
			else
			{
				//敬遠時//
				bot_iw = calc(i, t, w, s, b1, 1, 0, b1 + 1, b2, 1);

				//投球時//
				bot_batting = omote[b1][0] * calc(i, t, w + 1, s, 9, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 0, b1 + 1, b2, 1)
					+ omote[b1][2] * calc(i, t, w, s + 1, 9, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][3] * calc(i, t, w, s + 1, 9, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][4] * calc(i, t, w, s + 2, 9, 0, 0, b1 + 1, b2, 1) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 0, b1 + 1, b2, 1);

				bot_bant = omote[b1][7] * calc(i, t, w + 1, s, 9, 0, 1, b1 + 1, b2, 1) 
					+ (1.0 - omote[b1][7]) * calc(i, t, w + 1, s, b1, 0, 0, b1 + 1, b2, 1);

				top_iw = calc(i, t, w, s, b1, 1, 0, b1 + 1, b2, 0);

				top_batting = omote[b1][0] * calc(i, t, w + 1, s, 9, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 0, b1 + 1, b2, 0)
					+ omote[b1][2] * calc(i, t, w, s + 1, 9, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][3] * calc(i, t, w, s + 1, 9, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][4] * calc(i, t, w, s + 2, 9, 0, 0, b1 + 1, b2, 0) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 0, b1 + 1, b2, 0);

				top_bant = omote[b1][7] * calc(i, t, w + 1, s, 9, 0, 1, b1 + 1, b2, 0) 
					+ (1.0 - omote[b1][7]) * calc(i, t, w + 1, s, b1, 0, 0, b1 + 1, b2, 0);

				if (w == 0 || w == 1)
				{
					if (bot_iw > bot_batting && bot_iw > bot_bant){ //守備チーム後攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (bot_iw < max({bot_batting, bot_bant})){ //守備チーム後攻が投球を選択//
						if (top_batting >= top_bant){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];	
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
					else{ //守備チーム後攻にとって敬遠、投球は同じ勝率なので、先攻チームの勝率を下げる戦略を選択//
						if (top_iw <= max({top_batting, top_bant})){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else {
							if (top_batting >= top_bant){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];			
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
						}
					}
				}
				else //w==2//
				{
					if (bot_iw > bot_batting){ //守備チーム後攻は敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (bot_iw < bot_batting){ //守備チーム後攻は投球を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{ //守備チーム後攻にとって敬遠、投球は同じ勝率なので、先攻チームの勝率を下げる戦略を選ぶ//
						if (top_iw <= top_batting){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];							
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
				}
			}
		}
		else{
			if (w == 3) {
				stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
				stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
				table[i][t][w][s][r1][r2][r3][b1][b2][0] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, 0);
				table[i][t][w][s][r1][r2][r3][b1][b2][1] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, 1);
				return table[i][t][w][s][r1][r2][r3][b1][b2][m];
			}
			else
			{
				//敬遠時//
				top_iw = calc(i, t, w, s, b2, 1, 0, b1, b2 + 1, 1 - t);

				//投球時//
				top_batting = ura[b2][0] * calc(i, t, w + 1, s, 9, 1, 0, b1, b2 + 1, 0) 
					+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 0, b1, b2 + 1, 0)
					+ ura[b2][2] * calc(i, t, w, s - 1, 9, 1, 0, b1, b2 + 1, 0) 
					+ ura[b2][3] * calc(i, t, w, s - 1, 9, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][4] * calc(i, t, w, s - 2, 9, 0, 0, b1, b2 + 1, 0) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 0, b1, b2 + 1, 0);

				top_bant = ura[b2][7] * calc(i, t, w + 1, s, 9, 0, 1, b1, b2 + 1, 0) 
					+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 0, 0, b1, b2 + 1, 0);

				bot_iw = calc(i, t, w, s, b2, 1, 0, b1, b2 + 1, t);

				bot_batting = ura[b2][0] * calc(i, t, w + 1, s, 9, 1, 0, b1, b2 + 1, 1) 
					+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 0, b1, b2 + 1, 1)
					+ ura[b2][2] * calc(i, t, w, s - 1, 9, 1, 0, b1, b2 + 1, 1) 
					+ ura[b2][3] * calc(i, t, w, s - 1, 9, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][4] * calc(i, t, w, s - 2, 9, 0, 0, b1, b2 + 1, 1) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 0, b1, b2 + 1, 1);

				bot_bant = ura[b2][7] * calc(i, t, w + 1, s, 9, 0, 1, b1, b2 + 1, 1) 
					+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 0, 0, b1, b2 + 1, 1);

				if (w == 0 || w == 1)
				{
					if (top_iw > top_batting && top_iw > top_bant){ //守備チーム先攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];

					}
					else if (top_iw < max({top_batting, top_bant})){ //守備チーム先攻が投球を選択//
						if (top_batting >= top_bant){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;	
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
					else { //守備チーム先攻にとって敬遠、投球は同じ勝率なので、後攻チームの勝率を下げる戦略を選択//
						if (bot_iw <= max({bot_batting, bot_bant})){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else {
							if (top_batting >= top_bant) {
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else {
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
						}
					}
				}
				else{ //w==2//
					if (top_iw > top_batting){ //守備チーム先攻は敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_iw < top_batting){ //守備チーム先攻は投球を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{ //守備チーム先攻にとって敬遠、投球は同じ勝率なので、後攻チームの勝率を下げる戦略を選ぶ//
						if (bot_iw <= bot_batting){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
				}
			}
		}
	}

	/*ランナー3塁   盗塁なし   凡退、単打、二塁打、三塁打、本塁打、四球の順*/
	else if (r1 == 9 && r2 == 0 && r3 == 1)
	{
		if (t == 0){
			if (w == 3) {
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return  table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}

			else
			{
				//敬遠時//
				bot_iw = calc(i, t, w, s, b1, 0, 1, b1 + 1, b2, 1);

				//投球時//
				bot_batting = omote[b1][0] * calc(i, t, w + 1, s, 9, 0, 1, b1 + 1, b2, 1) 
					+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 0, b1 + 1, b2, 1)
					+ omote[b1][2] * calc(i, t, w, s + 1, 9, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][3] * calc(i, t, w, s + 1, 9, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][4] * calc(i, t, w, s + 2, 9, 0, 0, b1 + 1, b2, 1) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 0, 1, b1 + 1, b2, 1);

				bot_bant = omote[b1][7] * calc(i, t, w + 1, s + 1, 9, 0, 0, b1 + 1, b2, 1) 
					+ (1.0 - omote[b1][7])*calc(i, t, w + 1, s, b1, 0, 0, b1 + 1, b2, 1);

				top_iw = calc(i, t, w, s, b1, 0, 1, b1 + 1, b2, 0);

				top_batting = omote[b1][0] * calc(i, t, w + 1, s, 9, 0, 1, b1 + 1, b2, 0) 
					+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 0, b1 + 1, b2, 0)
					+ omote[b1][2] * calc(i, t, w, s + 1, 9, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][3] * calc(i, t, w, s + 1, 9, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][4] * calc(i, t, w, s + 2, 9, 0, 0, b1 + 1, b2, 0) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 0, 1, b1 + 1, b2, 0);

				top_bant = omote[b1][7] * calc(i, t, w + 1, s + 1, 9, 0, 0, b1 + 1, b2, 0) 
					+ (1.0 - omote[b1][7])*calc(i, t, w + 1, s, b1, 0, 0, b1 + 1, b2, 0);


				if (w == 0 || w == 1)
				{
					if (bot_iw > max({bot_batting, bot_bant})){ //守備チーム後攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (bot_iw < max({bot_batting, bot_bant})){ //守備チーム後攻が投球を選択//
						if (top_batting >= top_bant){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
					else{ //守備チーム後攻にとって敬遠、投球は同じ勝率なので、先攻チームの勝率を下げる戦略を選択
						if (top_iw <= max({top_batting, top_bant})){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else {
							if (top_batting >= top_bant){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];			
							}
						}
					}

				}
				else //w==2//
				{				
					if (top_iw <= top_batting){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else {
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					} 	
				}
			}
		}
		else //t==1//
		{
			if (w == 3) {
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}
			else
			{
				//敬遠時//
				top_iw = calc(i, t, w, s, b2, 0, 1, b1, b2 + 1, 0);

				top_batting = ura[b2][0] * calc(i, t, w + 1, s, 9, 0, 1, b1, b2 + 1, 0) 
					+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 0, b1, b2 + 1, 0)
					+ ura[b2][2] * calc(i, t, w, s - 1, 9, 1, 0, b1, b2 + 1, 0) 
					+ ura[b2][3] * calc(i, t, w, s - 1, 9, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][4] * calc(i, t, w, s - 2, 9, 0, 0, b1, b2 + 1, 0) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 0, 1, b1, b2 + 1, 0);

				top_bant = ura[b2][7] * calc(i, t, w + 1, s - 1, 9, 0, 0, b1, b2 + 1, 0) 
					+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 0, 0, b1, b2 + 1, 0);

				bot_iw = calc(i, t, w, s, b2, 0, 1, b1, b2 + 1, 1);

				bot_batting = ura[b2][0] * calc(i, t, w + 1, s, 9, 0, 1, b1, b2 + 1, 1) 
					+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 0, b1, b2 + 1, 1)
					+ ura[b2][2] * calc(i, t, w, s - 1, 9, 1, 0, b1, b2 + 1, 1) 
					+ ura[b2][3] * calc(i, t, w, s - 1, 9, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][4] * calc(i, t, w, s - 2, 9, 0, 0, b1, b2 + 1, 1) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 0, 1, b1, b2 + 1, 1);

				bot_bant = ura[b2][7] * calc(i, t, w + 1, s - 1, 9, 0, 0, b1, b2 + 1, 1) 
					+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 0, 0, b1, b2 + 1, 1);

				if (w == 0 || w == 1)
				{
					if (top_iw > max({top_batting, top_bant})){ //守備チーム先攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];

					}
					else if (top_iw < max({top_batting, top_bant})){ //守備チーム先攻が投球を選択//
						if (top_batting >= top_bant){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							} 
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
					else { //守備チーム先攻にとって敬遠、投球は同じ勝率なので、後攻チームの勝率を下げる戦略を選択//
						if (bot_iw <= max({bot_batting, bot_bant})){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						} 
						else {
							if (top_batting >= top_bant) {
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else {
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];	
							}
						}
					}
				}
				else //w==2//
				{
					if (top_iw > top_batting){ //守備チーム先攻は敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_iw < top_batting){ //守備チーム先攻は投球を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{ //守備チーム先攻にとって敬遠、投球は同じ勝率なので、後攻チームの勝率を下げる戦略を選ぶ//
						if (bot_iw <= bot_batting){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
				}
			}
		}
	}

	// ランナー1,2塁    　盗塁なし   凡退、単打、二塁打、三塁打、本塁打、四球の順//
	else if (r1 != 9 && r2 == 1 && r3 == 0)
	{
		if (t == 0){
			if (w == 3) {
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return  table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}
			else
			{
				//敬遠時//
				bot_iw = calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 1);

				bot_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s, r1, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 1) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 1)
					+ omote[b1][8] * calc(i, t, w + 2, s, 9, 0, 1, b1 + 1, b2, 1);

				bot_bant = omote[b1][7] * calc(i, t, w + 1, s, 9, 1, 1, b1 + 1, b2, 1) 
					+ (1.0 - omote[b1][7])*calc(i, t, w + 1, s, b1, 1, 0, b1 + 1, b2, 1);

				top_iw = calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 0);

				top_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s, r1, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 0) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 0)
					+ omote[b1][8] * calc(i, t, w + 2, s, 9, 0, 1, b1 + 1, b2, 0);

				top_bant = omote[b1][7] * calc(i, t, w + 1, s, 9, 1, 1, b1 + 1, b2, 0) 
					+ (1.0 - omote[b1][7])*calc(i, t, w + 1, s, b1, 1, 0, b1 + 1, b2, 0);

				if (w == 0 || w == 1)
				{
					if(bot_iw > max({bot_batting, bot_bant})){ //守備チーム後攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];			
					}
					else if (bot_iw < max({bot_batting, bot_bant})){ //守備チーム後攻が投球を選択//
						if (top_batting >= top_bant){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
					else{ //守備チームにとっては敬遠、投球どちらでも同じ勝率なので、先攻の勝率を下げる戦略を選択//
						if (top_iw <= max({top_batting, top_bant})){ //守備チームは敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];						
						}
						else{ //守備チームは投球を選択//
							if (top_batting >= top_bant){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
						}
					}
				}
				else //w==2//
				{
					bot_batting = omote[b1][0] * calc(i, t, w + 1, s, r1, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 1);

					top_batting = omote[b1][0] * calc(i, t, w + 1, s, r1, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 0);

					if(bot_iw > bot_batting){ //守備チーム後攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (bot_iw < bot_batting){ //守備チーム後攻が投球を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{ //守備チーム後攻にとって投球、敬遠は変わりがないので、先攻チームの勝率を下げる戦略を選ぶ//
						if (top_iw <= top_batting){ //敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //投球を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
				}
			}
		}
		else //t==1//
		{
			if (w == 3) {
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return  table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}
			else
			{
				//敬遠時//
				top_iw = calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 0);

				top_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s, r1, 1, 0, b1, b2 + 1, 0) 
					+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 0) 
					+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 0) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 0)
					+ ura[b2][8] * calc(i, t, w + 2, s, 9, 0, 1, b1, b2 + 1, 0);

				top_bant = ura[b2][7] * calc(i, t, w + 1, s, 9, 1, 1, b1, b2 + 1, 0) 
					+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 1, 0, b1, b2 + 1, 0);

				bot_iw = calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 1);

				bot_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s, r1, 1, 0, b1, b2 + 1, 1) 
					+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 1) 
					+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 1) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 1)
					+ ura[b2][8] * calc(i, t, w + 2, s, 9, 0, 1, b1, b2 + 1, 1);

				bot_bant = ura[b2][7] * calc(i, t, w + 1, s, 9, 1, 1, b1, b2 + 1, 1) 
					+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 1, 0, b1, b2 + 1, 1);

				if (w == 0 || w == 1)
				{
					if(top_iw > max({top_batting, top_bant})){ //守備チーム先攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;						
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_iw < max({top_batting, top_bant})){ //守備チーム先攻が投球を選択//
						if (bot_batting >= bot_bant){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];				
						}
					}
					else{ //守備チーム先攻にとっては敬遠、投球どちらでも同じ勝率なので、後攻の勝率を下げる戦略を選択//
						if (bot_iw <= max({bot_batting, bot_bant})){ //守備チームは敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //守備チーム先攻は投球を選択//
							if (bot_batting >= bot_bant){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
						}
					}
				}
				else //w==2//
				{
					top_batting = ura[b2][0] * calc(i, t, w + 1, s, r1, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 0);

					bot_batting = ura[b2][0] * calc(i, t, w + 1, s, r1, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 1);

					if(top_iw > top_batting){ //守備チーム先攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_iw < top_batting){ //守備チーム先攻が投球を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{ //守備チーム先攻にとって投球、敬遠は変わりがないので、後攻チームの勝率を下げる戦略を選ぶ//
						if (bot_iw <= bot_batting){ //敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //投球を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
				}
			}
		}
	}

	// ランナー2,3塁    盗塁なし   凡退、単打、二塁打、三塁打、本塁打、四球の順 //
	else if (r1 == 9 && r2 == 1 && r3 == 1)
	{
		if (t == 0){
			if (w == 3) {
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return  table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}

			else //w==0,1,2//
			{
				//敬遠時//
				bot_iw = calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 1);

				//投球時//
				bot_batting = omote[b1][0] * calc(i, t, w + 1, s, 9, 1, 1, b1 + 1, b2, 1) 
					+ omote[b1][1] * calc(i, t, w, s + 2, b1, 0, 0, b1 + 1, b2, 1)
					+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 1) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 1);

				bot_bant = omote[b1][7] * calc(i, t, w + 1, s + 1, 9, 0, 1, b1 + 1, b2, 1) 
					+ (1.0 - omote[b1][7]) * calc(i, t, w + 1, s, b1, 0, 1, b1 + 1, b2, 1);

				top_iw = calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 0);

				top_batting = omote[b1][0] * calc(i, t, w + 1, s, 9, 1, 1, b1 + 1, b2, 0) 
					+ omote[b1][1] * calc(i, t, w, s + 2, b1, 0, 0, b1 + 1, b2, 0)
					+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 0) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 0);

				top_bant = omote[b1][7] * calc(i, t, w + 1, s + 1, 9, 0, 1, b1 + 1, b2, 0) 
					+ (1.0 - omote[b1][7]) * calc(i, t, w + 1, s, b1, 0, 1, b1 + 1, b2, 0);

				if (w == 0 || w == 1)
				{
					if(bot_iw > max({bot_batting, bot_bant})){ //守備チーム後攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (bot_iw < max({bot_batting, bot_bant})){ //守備チーム後攻が投球を選択//
						if (top_batting >= top_bant){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];	
						}
						else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;	
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];				
						}
					}
					else{ //守備チームにとっては敬遠、投球どちらでも同じ勝率なので、先攻の勝率を下げる戦略を選択
						if (top_iw <= max({top_batting, top_bant})){ //守備チームは敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //守備チームは投球を選択//
							if (top_batting >= top_bant){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];							
							}
						}
					}
				}
				else
				{
					if(bot_iw > bot_batting){ //守備チーム後攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (bot_iw < bot_batting){ //守備チーム後攻が投球を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{ //守備チーム後攻にとって投球、敬遠は変わりがないので、先攻チームの勝率を下げる戦略を選ぶ//
						if (top_iw <= top_batting){ //敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //投球を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
				}
			}
		}
		else //t==1//
		{
			if (w == 3){
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return  table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}

			else
			{
				//敬遠時//
				top_iw = calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 0);

				//投球時//
				top_batting = ura[b2][0] * calc(i, t, w + 1, s, 9, 1, 1, b1, b2 + 1, 0) 
					+ ura[b2][1] * calc(i, t, w, s - 2, b2, 0, 0, b1, b2 + 1, 0)
					+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 0) 
					+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 0) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 0);

				top_bant = ura[b2][7] * calc(i, t, w + 1, s - 1, 9, 0, 1, b1, b2 + 1, 0) 
					+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 0, 1, b1, b2 + 1, 0);

				bot_iw = calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 1);
								
				bot_batting = ura[b2][0] * calc(i, t, w + 1, s, 9, 1, 1, b1, b2 + 1, 1) 
					+ ura[b2][1] * calc(i, t, w, s - 2, b2, 0, 0, b1, b2 + 1, 1)
					+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 1) 
					+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 1) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 1);

				bot_bant = ura[b2][7] * calc(i, t, w + 1, s - 1, 9, 0, 1, b1, b2 + 1, 1) 
					+ (1.0 - ura[b2][7]) * calc(i, t, w + 1, s, b2, 0, 1, b1, b2 + 1, 1);
				
				if (w == 0 || w == 1)
				{					
					if(top_iw > top_batting && top_iw > top_bant){ //守備チーム先攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_iw < max({top_batting, top_bant})){ //守備チーム先攻が投球を選択//
						if (top_batting >= top_bant){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];			
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
					else{ //守備チームにとっては敬遠、投球どちらでも同じ勝率なので、後攻の勝率を下げる戦略を選択//
						if (bot_iw <= max({bot_batting, bot_bant})){ //守備チームは敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;			
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //守備チームは投球を選択//
							if (top_batting >= top_bant){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];			
							}
						}
					}
				}
				else //w==2//
				{
					if(top_iw > top_batting){ //守備チーム先攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_iw < top_batting){ //守備チーム先攻が投球を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{ //守備チーム先攻にとって投球、敬遠は変わりがないので、後攻チームの勝率を下げる戦略を選ぶ//
						if (bot_iw <= bot_batting){ //敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //投球を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
				}
			}
		}
	}

	/*ランナー1,3塁  凡退、単打、二塁打、三塁打、本塁打、四球の順*/
	else if (r1 != 9 && r2 == 0 && r3 == 1)
	{
		if (t == 0){
			if (w == 3)  {
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}

			else
			{
				//敬遠時//
				bot_iw = calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 1);

				//投球時//
				bot_steal = omote[b1][6] * calc(i, t, w, s, 9, 1, 1, b1, b2, 1) 
					+ (1 - omote[b1][6]) * calc(i, t, w + 1, s, 9, 0, 1, b1, b2, 1);

				bot_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s, r1, 0, 1, b1 + 1, b2, 1) 
					+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 1) 
					+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 1)
					+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 1) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 1)
					+ omote[b1][8] * calc(i, t, w + 2, s+1, 9, 0, 0, b1 + 1, b2, 1);

				bot_bant = omote[b1][7] * calc(i, t, w + 1, s + 1, 9, 1, 0, b1 + 1, b2, 1) 
					+ (1.0 - omote[b1][7]) * calc(i, t, w + 1, s, b1, 1, 0, b1 + 1, b2, 1);

				top_iw = calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 0);

				top_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s, r1, 0, 1, b1 + 1, b2, 0) 
					+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 0) 
					+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 0)
					+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 0) 
					+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 0)
					+ omote[b1][8] * calc(i, t, w + 2, s+1, 9, 0, 0, b1 + 1, b2, 0);

				top_steal = omote[b1][6] * calc(i, t, w, s, 9, 1, 1, b1, b2, 0) 
					+ (1 - omote[b1][6]) * calc(i, t, w + 1, s, 9, 0, 1, b1, b2, 0);

				top_bant = omote[b1][7] * calc(i, t, w + 1, s + 1, 9, 1, 0, b1 + 1, b2, 0) 
					+ (1.0 - omote[b1][7]) * calc(i, t, w + 1, s, b1, 1, 0, b1 + 1, b2, 0);

				if (w == 0 || w == 1)
				{
					if(bot_iw > bot_batting && bot_iw >  bot_steal && bot_iw >  bot_bant){ //守備チーム後攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (bot_iw < max({bot_batting, bot_steal, bot_bant})){ //守備チーム後攻が投球を選択//
						if (top_batting == max({top_batting, top_steal, top_bant})){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else if (top_steal == max({top_batting, top_steal, top_bant})){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'S';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];				
						}
					}
					else{ //守備チーム後攻にとって投球、敬遠は変わりがないので、先攻チームの勝率を下げる戦略を選ぶ
						if(top_iw <= max({top_batting, top_steal, top_bant})){ //敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //投球を選択//
							if (top_batting == max({top_batting, top_steal, top_bant})){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else if (top_steal == max({top_batting, top_steal, top_bant})){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'S';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
						}
					} 
				}
				else //w==2//
				{
					bot_batting = omote[b1][0] * calc(i, t, w + 1, s, r1, 0, 1, b1 + 1, b2, 1) 
						+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 1);

					top_batting = omote[b1][0] * calc(i, t, w + 1, s, r1, 0, 1, b1 + 1, b2, 0) 
						+ omote[b1][1] * calc(i, t, w, s + 1, b1, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][2] * calc(i, t, w, s + 2, 9, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][3] * calc(i, t, w, s + 2, 9, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][4] * calc(i, t, w, s + 3, 9, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][5] * calc(i, t, w, s, b1, 1, 1, b1 + 1, b2, 0);
					
					if(bot_iw > bot_steal && bot_iw > bot_batting){ //守備チーム後攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (bot_iw < max({bot_steal, bot_batting})){ //守備チーム後攻が投球を選択//
						if (top_steal > top_batting){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'S';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];				
						}
					}
					else{ //守備チーム後攻にとって投球、敬遠は変わりがないので、先攻チームの勝率を下げる戦略を選ぶ//
						if(top_iw <= max({top_steal, top_batting})){ //敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //投球を選択//
							if (top_steal > top_batting){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'S';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];									
							}
						}
					}
				}
			}
		}
		else //t==1//
		{
			if (w == 3) {
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}
			else
			{
				//敬遠時//
				top_iw = calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 0);
				//投球時//
				top_steal = ura[b2][6] * calc(i, t, w, s, 9, 1, 1, b1, b2, 0) 
					+ (1 - ura[b2][6]) * calc(i, t, w + 1, s, 9, 0, 1, b1, b2, 0);

				top_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s, r1, 0, 1, b1, b2 + 1, 0) 
					+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 0) 
					+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 0)
					+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 0) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 0)
					+ ura[b2][8] * calc(i, t, w + 2, s-1, 9, 0, 0, b1, b2 + 1, 0);

				top_bant = ura[b2][7] * calc(i, t, w + 1, s - 1, 9, 1, 0, b1, b2 + 1, 0) 
					+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 1, 0, b1, b2 + 1, 0);

				bot_iw = calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 1);

				bot_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s, r1, 0, 1, b1, b2 + 1, 1) 
					+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 1) 
					+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 1)
					+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 1) 
					+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 1)
					+ ura[b2][8] * calc(i, t, w + 2, s-1, 9, 0, 0, b1, b2 + 1, 1);

				bot_steal = ura[b2][6] * calc(i, t, w, s, 9, 1, 1, b1, b2, 1) 
					+ (1 - ura[b2][6])*calc(i, t, w + 1, s, 9, 0, 1, b1, b2, 1);

				bot_bant = ura[b2][7] * calc(i, t, w + 1, s - 1, 9, 1, 0, b1, b2 + 1, 1) 
					+ (1.0 - ura[b2][7]) * calc(i, t, w + 1, s, b2, 1, 0, b1, b2 + 1, 1);

				if (w == 0 || w==1){
					if(top_iw > top_batting && top_iw > top_steal && top_iw > top_bant){ //守備チーム先攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_iw < max({top_batting, top_steal, top_bant})){ //守備チーム先攻が投球を選択//
						if (bot_batting == max({bot_batting, bot_steal, bot_bant})){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else if (bot_steal == max({bot_batting, bot_steal, bot_bant})){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'S';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];	
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];			
						}
					}
					else{ //守備チーム先攻にとって投球、敬遠は変わりがないので、後攻チームの勝率を下げる戦略を選ぶ//
						if(bot_iw <= max({bot_batting, bot_steal, bot_bant})){ //敬遠を選択//
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //投球を選択//
							if (bot_batting == max({bot_batting, bot_steal, bot_bant})){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else if (bot_steal == max({bot_batting, bot_steal, bot_bant})){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'S';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							} 				
						}
					} 
				}
				else //w==2//
				{
					top_batting = ura[b2][0] * calc(i, t, w + 1, s, r1, 0, 1, b1, b2 + 1, 0) 
						+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 0);

					bot_batting = ura[b2][0] * calc(i, t, w + 1, s, r1, 0, 1, b1, b2 + 1, 1) 
						+ ura[b2][1] * calc(i, t, w, s - 1, b2, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][2] * calc(i, t, w, s - 2, 9, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][3] * calc(i, t, w, s - 2, 9, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][4] * calc(i, t, w, s - 3, 9, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][5] * calc(i, t, w, s, b2, 1, 1, b1, b2 + 1, 1);
					
					if(top_iw > top_steal && top_iw > top_batting){ //守備チーム先攻が敬遠を選択//
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else if (top_iw < max({top_steal, top_batting})){ //守備チーム後攻が投球を選択//
						if (bot_steal > bot_batting){
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'S';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
							table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
							table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
					}
					else{ //守備チーム先攻にとって投球、敬遠は変わりがないので、後攻チームの勝率を下げる戦略を選ぶ//
						if(bot_iw <= bot_steal && bot_iw <= bot_batting){ //敬遠を選択//			
							stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'W';
							return table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_iw;
							stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'N';
							return table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_iw;
							return table[i][t][w][s][r1][r2][r3][b1][b2][m];
						}
						else{ //投球を選択//
							if (bot_steal > bot_batting){
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_steal;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'S';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_steal;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];
							}
							else{
								stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
								table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
								stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
								table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
								return table[i][t][w][s][r1][r2][r3][b1][b2][m];				
							}
						}
					}
				}
			}
		}
	}

	/*ランナー満塁   盗塁なし 敬遠無し 凡退、単打、二塁打、三塁打、本塁打、四球の順*/
	else if (r1 != 9 && r2 == 1 && r3 == 1)
	{
		if (t == 0){
			if (w == 3)
			{
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return  table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}
			else
			{
	
				if (w == 0 || w == 1)
				{
					bot_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s, r1, 1, 1, b1 + 1, b2, 1) 
						+ omote[b1][1] * calc(i, t, w, s + 2, b1, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][2] * calc(i, t, w, s + 3, 9, 1, 0, b1 + 1, b2, 1)
						+ omote[b1][3] * calc(i, t, w, s + 3, 9, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][4] * calc(i, t, w, s + 4, 9, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][5] * calc(i, t, w, s + 1, b1, 1, 1, b1 + 1, b2, 1)
						+ omote[b1][8] * calc(i, t, w + 2, s, 9, 1, 1, b1 + 1, b2, 1);

					bot_bant = omote[b1][7] * calc(i, t, w + 1, s + 1, 9, 1, 1, b1 + 1, b2, 1) 
						+ (1.0 - omote[b1][7])*calc(i, t, w + 1, s, b1, 1, 1, b1 + 1, b2, 1);

					top_batting = (omote[b1][0]-omote[b1][8]) * calc(i, t, w + 1, s, r1, 1, 1, b1 + 1, b2, 0) 
						+ omote[b1][1] * calc(i, t, w, s + 2, b1, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][2] * calc(i, t, w, s + 3, 9, 1, 0, b1 + 1, b2, 0)
						+ omote[b1][3] * calc(i, t, w, s + 3, 9, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][4] * calc(i, t, w, s + 4, 9, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][5] * calc(i, t, w, s + 1, b1, 1, 1, b1 + 1, b2, 0)
						+ omote[b1][8] * calc(i, t, w + 2, s, 9, 1, 1, b1 + 1, b2, 0);

					top_bant = omote[b1][7] * calc(i, t, w + 1, s + 1, 9, 1, 1, b1 + 1, b2, 0) 
						+ (1.0 - omote[b1][7])*calc(i, t, w + 1, s, b1, 1, 1, b1 + 1, b2, 0);

					if (top_batting >= top_bant){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;	
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'B';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];				
					}
				}
				else //w==2//
				{
					bot_batting = omote[b1][0] * calc(i, t, w + 1, s, r1, 1, 1, b1 + 1, b2, 1) 
						+ omote[b1][1] * calc(i, t, w, s + 2, b1, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][2] * calc(i, t, w, s + 3, 9, 1, 0, b1 + 1, b2, 1) 
						+ omote[b1][3] * calc(i, t, w, s + 3, 9, 0, 1, b1 + 1, b2, 1)
						+ omote[b1][4] * calc(i, t, w, s + 4, 9, 0, 0, b1 + 1, b2, 1) 
						+ omote[b1][5] * calc(i, t, w, s + 1, b1, 1, 1, b1 + 1, b2, 1);

					top_batting = omote[b1][0] * calc(i, t, w + 1, s, r1, 1, 1, b1 + 1, b2, 0) 
						+ omote[b1][1] * calc(i, t, w, s + 2, b1, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][2] * calc(i, t, w, s + 3, 9, 1, 0, b1 + 1, b2, 0) 
						+ omote[b1][3] * calc(i, t, w, s + 3, 9, 0, 1, b1 + 1, b2, 0)
						+ omote[b1][4] * calc(i, t, w, s + 4, 9, 0, 0, b1 + 1, b2, 0) 
						+ omote[b1][5] * calc(i, t, w, s + 1, b1, 1, 1, b1 + 1, b2, 0);
					
					//守備チーム後攻は投球を選択//
					stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'H';
					table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
					stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'P';
					table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
					return table[i][t][w][s][r1][r2][r3][b1][b2][m];		
				}
			}
		}
		else
		{
			if (w == 3) {
				stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
				return table[i][t][w][s][r1][r2][r3][b1][b2][m] = calc(i + 1, t + 1, 0, s, 9, 0, 0, b1, b2, m);
			}
			else
			{
				if (w == 0 || w == 1)
				{
					top_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s, r1, 1, 1, b1, b2 + 1, 0) 
						+ ura[b2][1] * calc(i, t, w, s - 2, b2, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][2] * calc(i, t, w, s - 3, 9, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][3] * calc(i, t, w, s - 3, 9, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][4] * calc(i, t, w, s - 4, 9, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][5] * calc(i, t, w, s - 1, b2, 1, 1, b1, b2 + 1, 0)
						+ ura[b2][8] * calc(i, t, w + 2, s, 9, 1, 1, b1, b2 + 1, 0);

					top_bant = ura[b2][7] * calc(i, t, w + 1, s - 1, 9, 1, 1, b1, b2 + 1, 0) 
						+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 1, 1, b1, b2 + 1, 0);

					bot_batting = (ura[b2][0]-ura[b2][8]) * calc(i, t, w + 1, s, r1, 1, 1, b1, b2 + 1, 1) 
						+ ura[b2][1] * calc(i, t, w, s - 2, b2, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][2] * calc(i, t, w, s - 3, 9, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][3] * calc(i, t, w, s - 3, 9, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][4] * calc(i, t, w, s - 4, 9, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][5] * calc(i, t, w, s - 1, b2, 1, 1, b1, b2 + 1, 1)
						+ ura[b2][8] * calc(i, t, w + 2, s, 9, 1, 1, b1, b2 + 1, 1);

					bot_bant = ura[b2][7] * calc(i, t, w + 1, s - 1, 9, 1, 1, b1, b2 + 1, 1) 
						+ (1.0 - ura[b2][7])*calc(i, t, w + 1, s, b2, 1, 1, b1, b2 + 1, 1);

					//守備チーム先攻は投球を選択//
					if (top_batting >= top_bant){
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];
					}
					else{
						stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
						table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_bant;
						stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'B';
						table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_bant;
						return table[i][t][w][s][r1][r2][r3][b1][b2][m];				
					}
				}
				else
				{
					top_batting = ura[b2][0] * calc(i, t, w + 1, s, r1, 1, 1, b1, b2 + 1, 0) 
						+ ura[b2][1] * calc(i, t, w, s - 2, b2, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][2] * calc(i, t, w, s - 3, 9, 1, 0, b1, b2 + 1, 0) 
						+ ura[b2][3] * calc(i, t, w, s - 3, 9, 0, 1, b1, b2 + 1, 0)
						+ ura[b2][4] * calc(i, t, w, s - 4, 9, 0, 0, b1, b2 + 1, 0) 
						+ ura[b2][5] * calc(i, t, w, s - 1, b2, 1, 1, b1, b2 + 1, 0);

					bot_batting = ura[b2][0] * calc(i, t, w + 1, s, r1, 1, 1, b1, b2 + 1, 1) 
						+ ura[b2][1] * calc(i, t, w, s - 2, b2, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][2] * calc(i, t, w, s - 3, 9, 1, 0, b1, b2 + 1, 1) 
						+ ura[b2][3] * calc(i, t, w, s - 3, 9, 0, 1, b1, b2 + 1, 1)
						+ ura[b2][4] * calc(i, t, w, s - 4, 9, 0, 0, b1, b2 + 1, 1) 
						+ ura[b2][5] * calc(i, t, w, s - 1, b2, 1, 1, b1, b2 + 1, 1);
					
					//守備チーム先攻が投球を選択//
					stable[i][t][w][s][r1][r2][r3][b1][b2][0] = 'P';
					table[i][t][w][s][r1][r2][r3][b1][b2][0] = top_batting;
					stable[i][t][w][s][r1][r2][r3][b1][b2][1] = 'H';
					table[i][t][w][s][r1][r2][r3][b1][b2][1] = bot_batting;
					return table[i][t][w][s][r1][r2][r3][b1][b2][m];				
				}
			}
		}
	}
	else {
		stable[i][t][w][s][r1][r2][r3][b1][b2][m] = 'N';
		return table[i][t][w][s][r1][r2][r3][b1][b2][m] = 0;
	}
}

int main(void)
{
	//time_t start, end;
	//double diff;

	/*先攻チームのデータ読み取り*/
	FILE *ff = fopen("data/top.txt", "r");

	for (int i = 0; i < NAME; i++){
		for (int j = 0; j < RESULT; j++)
		{
			fscanf(ff, "%lf", &omote[i][j]);
		}
	}    

	/*後攻チームのcsvデータ読み取り*/
	FILE *fg = fopen("data/bottom.txt", "r");

	for (int i = 0; i < NAME; i++){
		for (int j = 0; j < RESULT; j++)
		{
			fscanf(fg, "%lf", &ura[i][j]);
		}
	}	

	//start = clock();

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

	//printf("試合開始の先攻の勝率:%f\n", calc(0, 0, 0, 20, 9, 0, 0, 0, 0, 0));	//
	printf("試合開始の先攻の勝率:%f\n", calc(9, 1, 2, 30, 9, 0, 0, 0, 0, 0));

	//end = clock();
	//diff = (double)(end - start) / CLOCKS_PER_SEC;

	//printf("計算時間 %.3f sec\n", diff);

	return 0;
}
