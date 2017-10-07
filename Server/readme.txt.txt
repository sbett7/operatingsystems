After connecting to server

1. get username and password and send both to the server.
2. receive whether or not the credentials were bad.  If bad then close connection and stop. Confirmation is sent as a uint16_t integer so will need to use ntohs() to convert to readable format. 1 means correct credentials 0 means bad credentials.
3. If credentials are good then ask user for option between 1 and 3.  (1 - Hangman game, 2 - Leaderboard, 3 - Exit).  send as uint16_t integer with htons()
4. Implementing Hangman game.  Server will send the first word's length and second word's length as well as the max number of guesses.  all as uint16_t integers.
5. client then sends guess to server.  I have it set to send as a uint16_t integer because some weird stuff was happening.  So when you get the char, convert it to uint16_t before sending it to the server.
6. Server will send an array with all of the letter positions for both words where the guess was correct.  1 means that the guessed letter exists at this point, 0 means it doesn't.  the number 2 is used to indicate a space.
	so for example if the words were 	repair activity		the integer array for a guess for 'a' would be 000100210000000
7.  In order for the server to know when the game is won/lost, after finishing the word or running out of guesses, send an integer in uint16_t format.
	1 - GAME_LOST	-	Lost the game
	2 - GAME_WON 	- 	Won the game


8. User sends 2 when asked for a command
9. server will send a uint16_t that contains the number of clients on the leaderboard.
10. server sends the size of the username (sizeof(char *username)).  
11. server sends the username
12. server sends the number of games played
12. server sends the number of games won
13. server repeats 10-12 until the number provided by 9. is met.