import configparser
from s826board import S826Board, errhandle


config = configparser.ConfigParser()
configfile = 'testconfig.ini'
config.read(configfile)

test_S826Board = S826Board(config)
test_S826Board.configure()


test_S826Board.close()
