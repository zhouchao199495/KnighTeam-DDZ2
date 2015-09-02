#include "AlphaBeta.h"
#include "FileWriter.h"
#include "sstream"
#include "DDZMoveManager.h"
#include "DDZCombFactory.h"
#include "time.h"

AlphaBeta::AlphaBeta(void)
{
	junkoNum=0;
	bestMove = CARDSMOVE();
	FirstMoves = vector<CARDSMOVE>();
	initBestMoveLinkList();
	
};

AlphaBeta::~AlphaBeta(void)
{
}

void AlphaBeta::initBestMoveLinkList()
{
	for (int i = 0; i < BestMoveLinkListLen; i++)
	{
		BestMoveLinkList[i] = 0;
	}
}
int AlphaBeta::SearchAGoodMove(int nDepth)
{
	DDZMoveManager ddz_MM = DDZMoveManager();
	int SimulateTimes = 5;
	start = clock();
	DDZCombFactory ddz_CF = DDZCombFactory(Player::p3_EachCardNum, Player::p3_cardsNum);
	vector<CARDSMOVE> mustWin = ddz_CF.getComb1LeastSingle().moves;
	ddz_CF.setCarryCards1_1(&mustWin);
	ddz_CF.setCarryCards3(&mustWin);
	ddz_CF.setCarryCards4(&mustWin);
	

	while (true)
	{
		
		SearchAGoodMove(nDepth, -1);

		int i = getMaxScoreIndex(FirstMoves);
		if (FirstMoves.size() == 1)
		{
			return 0;
		}

		if (CanWin(mustWin, FirstMoves[0].outWay))
			return 0;

		
		if (i != -1)
		{
			BestMoveLinkList[i]++;
		}
		end = clock();
		if (end - start > (AlphaBetaTime * 1000))
		{
			break;
		}
	}
	
	

	int BestMoveIndex = getMaxTimesBestMove();//最佳走步链表中取最优走步
	if (BestMoveIndex != -1)
	{
		bestMove = FirstMoves[BestMoveIndex];
	}
	else
	{
		bestMove = FirstMoves[0];
	}
	return 0;
}

int AlphaBeta::getMaxTimesBestMove()
{
	int current = 0;
	int index = -1;
	for (int i = FirstMoves.size() - 1; i >= 0; i--)
	{
		if (BestMoveLinkList[i] >= current)
		{
			index = i;
			current = BestMoveLinkList[i];
		}
	}

	return index;
}
int AlphaBeta::SearchAGoodMove(int nDepth, int nodeIndex)
{
	DDZMoveManager ddz_MM = DDZMoveManager();




	int canPlayOver = 0;
	int current = -99999;
	CMoveGenerator *ddz_MG;
	CEveluation *ddz_Eve;
	ddz_MG = (CMoveGenerator*)malloc(sizeof(CMoveGenerator));
	ddz_Eve = (CEveluation*)malloc(sizeof(CEveluation));
	vector<CARDSMOVE> moves = vector<CARDSMOVE>();

	moves = ddz_MG->getMoves(nDepth);

	/*if (Player::p3_cardsNum == 20)
	{
		bestMove = ddz_Eve->firstAttack(moves);
		return 0;	
	}*/
	int result = IsGameOver(nDepth);
	if (result)
	{
		if (result > 0)
		{
			FirstMoves[nodeIndex].win += 1;
		}
		FirstMoves[nodeIndex].Count++;
		FirstMoves[nodeIndex].score += (result *(4000 - nDepth * 1000));//加入区分赢的深度，越浅越好
		return  0;
	}

	if (nDepth == AlphaBeta_Depth)
	{
		FirstMoves[nodeIndex].Count++;
		return 0;
	}

	//if(moves.size()>0&&moves[0].outWay==1)
	//{
	//	Player::p3_allMoves=moves;
	//}

	
	ddz_Eve->PositiveSimulatedMenu();
	ddz_Eve->EveluateMoves(&moves, nDepth);

	if (nDepth < 4)
	{
		if (nDepth == 0)
		{
			FirstMoves = moves;
			for (int i = FirstMoves.size() - 1; i >= 0; i--)
			{
				FirstMoves[i].Count=0;
				FirstMoves[i].win=0;
			}
			if (moves.size() == 1)
			{
				bestMove = moves[0];
				return 0;
			}
		}
		
		for (size_t i = 0; i < moves.size(); i++)
		{
			MakeMove(moves[i], nDepth);

			if (nDepth==0)
				SearchAGoodMove(nDepth + 1, i);
			else
				SearchAGoodMove(nDepth + 1, nodeIndex);

			UnMakeMove(moves[i], nDepth);
		}
		
	}
	else
	{
		int index = getMaxScoreIndex(moves);
		if (index != -1)
		{
			MakeMove(moves[index], nDepth);
			if (nDepth % 3 != 0)
			{
				ddz_Eve->ClearedByPlayCards(moves[index].cards);
				ddz_Eve->UpdateByPlayCards(nDepth % 3, moves[index].cardsType, moves[index].cards);
			}
		
			SearchAGoodMove(nDepth + 1, nodeIndex);
			UnMakeMove(moves[index], nDepth);
		}
	}


	//vector<CARDSMOVE> comb = ddz_MM.getCombBySocre();
	//ddz_MM.setMovesStatus(&comb);

	//// 判断是否达到必胜状态
	//Player::p3_canPlayOver = ddz_MM.canPlayOver(comb);
	//if (Player::p3_canPlayOver)
	//{
	//	Player::p3_comb = comb;
	//}

	/*for (size_t i = 0; i<moves.size(); i++)
	{
		if (nDepth == 0)
		{
			if (current<moves[i].score)
			{
				bestMove = moves[i];
				current = moves[i].score;
			}
		}
	}*/
	return 0;
}



int	AlphaBeta::getMaxScoreIndex(vector<CARDSMOVE> moves)
{
	int index = -1;
	int current = -9999;

	int winIndex = -1;
	double currentWin = 0;

	for (size_t i = 0; i < moves.size(); i++)
	{
		if ((moves[i].score) > current)
		{
			current = (moves[i].score);
			index = i;
		}

		if (moves[i].Count != 0 &&(double)moves[i].win / moves[i].Count >= currentWin)
		{
			currentWin = (double)moves[i].win / moves[i].Count;
			winIndex = i;
		}
	}

	if (currentWin >= 0.9)
		return winIndex;
	return index;
}
int AlphaBeta::IsFinal()
{
	if (IsFinal1LastOne())
	{
		return 1;
	}
	else if (IsFinal2LastTwo())
	{
		return 2;
	}
		
	else if (IsFinal3LastThree())
	{
		return 3;
	}
	else
	{
		return 4;
	}
		
}

/*
*   终局其他玩家只剩下一张牌
*/
bool AlphaBeta::IsFinal1LastOne()
{
	return IsFinalByLastCardsNum(1);
}

/*
*   终局其他玩家只剩下两张牌
*/
bool AlphaBeta::IsFinal2LastTwo()
{
	return IsFinalByLastCardsNum(2);
}

/*
*   终局其他玩家只剩下三张牌
*/
bool AlphaBeta::IsFinal3LastThree()
{
	return IsFinalByLastCardsNum(3);
}


bool AlphaBeta::IsFinalByLastCardsNum(int lastCardsNum)
{
	if ((Player::p1_IsLandlord && Player::p1_cardsNum == lastCardsNum) || (Player::p2_IsLandlord && Player::p2_cardsNum == lastCardsNum)
		|| Player::p3_IsLandlord && (Player::p1_cardsNum == lastCardsNum || Player::p2_cardsNum == lastCardsNum))
	{
		return true;
	}

	return false;
}

//CARDSMOVE firstAttack(vector<CARDSMOVE> moves)
//{
//	
//}