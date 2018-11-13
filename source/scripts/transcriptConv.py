import argparse
import re

class Othello:
    def __init__(self, out_file):
        self.board = []
        self.ResetBoard()
        self.file = open(out_file, 'w')
        self.WriteHeaders()
        
    def WriteHeaders(self):
        headers = ",".join("m_" + str(x) for x in list(range(0, 60)))
        headers = headers + ",player,e_move,round\n"
        self.file.write(headers)

    def WriteBoard(self, move, player, round):
        board_info = ",".join(str(x) for x in self.board)
        board_info = board_info + ",{},{},{}\n".format(player, move, round) 
        self.file.write(board_info)

    def CloseOutFile(self): 
        self.file.close()
    
    def ResetBoard(self):
        self.board.clear()

        self.board = [0]*64
        self.board[ToIndex('D4')] = -1
        self.board[ToIndex('E5')] = -1
        self.board[ToIndex('E4')] = 1
        self.board[ToIndex('D5')] = 1

    def PrintBoard(self):
        for i in range(len(self.board)):
            if i % 8 == 0:
                print()
            else: print("{:3}".format(self.board[i]), end="")
        print()
        print()

    def GetTile(self, pos):
        return self.board[pos]
    
    def DoMove(self, move, player):

        flip_list = []
        
        flip_list = flip_list + self.CheckDirection(move, player, 0, -1) # Check Up
        flip_list = flip_list + self.CheckDirection(move, player, 0, 1) # Check Down
        flip_list = flip_list + self.CheckDirection(move, player, -1, 0) # Check Left
        flip_list = flip_list + self.CheckDirection(move, player, 1, 0) # Check Right
        flip_list = flip_list + self.CheckDirection(move, player, 1, -1) # Check Upper Right
        flip_list = flip_list + self.CheckDirection(move, player, -1, -1) # Check Upper Left
        flip_list = flip_list + self.CheckDirection(move, player, 1, 1) # Check Lower Right
        flip_list = flip_list + self.CheckDirection(move, player, -1, 1) # Check Lower Left

        for tile in flip_list: self.board[tile] *= (-1)
        self.board[move] = player
        
    def CheckDirection(self, move, player, x_offset, y_offset):
        dir_pos = move
        flip_list = []
        while(True):
            
            if dir_pos % 8 == 0 and x_offset == -1: break
            if dir_pos % 8 == 7 and x_offset == 1: break

            dir_pos += x_offset + y_offset * 8

            if dir_pos > 63 or dir_pos < 0: break

            if self.GetTile(dir_pos) == player * (-1): flip_list.append(dir_pos)
            elif self.GetTile(dir_pos) == player:
                if len(flip_list) > 0: return flip_list
                else: return[]
            else:
                return []
        return []

def ToIndex(xy_move):
    index = (ord(xy_move[0]) - ord('A')) + (int(xy_move[1]) - 1) * 8
    return index

def GetRawTranscript(filename):
    file_ = open(filename)
    data = file_.read()
    file_.close()

    raw = data.split("*]")[1]
    raw = raw.strip("\n, ), ;")
    return raw

def ConvertPlayer(player_letter):
    if player_letter == 'B':
        return 1
    else: return -1

def ConvertToBoards(rawTS, out_file):
    rawTS = rawTS.replace('B[PA]', "").replace('W[PA]', "")
    
    player_list = [ConvertPlayer(x) for x in re.findall(r"[\w']+", rawTS)[::2]]
    move_list = [ToIndex(x) for x in re.findall(r"[\w']+", rawTS)[1::2]]
    
    if len(move_list) != 60:
        print("ERROR: Game ended before 60 moves were made")
        exit(-1)

    game = Othello(out_file)

    for i in range(len(move_list)):
        game.WriteBoard(move_list[i], player_list[i], i)
        game.DoMove(move_list[i], player_list[i])
        
        
    game.CloseOutFile()



if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", type=str)
    parser.add_argument("out_file", type=str)
    args = parser.parse_args()

    rawTS = GetRawTranscript(args.filename)
    ConvertToBoards(rawTS, args.out_file)