# Tetris

This is a working implementation of Tetris, with all central rules in play, written entirely in C. 
It uses the [Allego game programming library](http://www.liballeg.org) for managing graphics and player
input. 

The core rules of the game are there, but it is lacking many of the modern updates (a hold option,
e.g.) to the game. My goal here was not to reinvent the wheel, but rather to create a project that 
would force me to research and become comfortable with a new C library, while reinforcing fundamental 
C concepts (structs, pointers, memory management, etc.).

---

##### How do I play?

Great question! Creating a shareable binary file that can run the game is still a work in progress.
The readme and repo will be updated when it is available. For now, you will have to compile yourself.

To do so, first download and install Allegro v5. Then, you will need to use a custom make file to 
ensure the proper header files are accessible. I have included my makefile, which should work for
Mac systems. Additional information will be available on the Allegro forums if you are unable to get
the game running, and feel free to leave a comment if you have any questions!

##### I have thoughts on how to improve this.

Wonderful! I am delighted to receive any and all feedback, so feel free to make a pull request or leave
a comment with any thoughts you might have. 
