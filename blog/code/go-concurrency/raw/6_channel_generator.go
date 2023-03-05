package main

import (
	"fmt"
	"math/rand"
	"time"
)

func boring(id int, msg string) <-chan string {
	c := make(chan string)
	go func() {
		for i := 0; ; i++ {
			msg := fmt.Sprintf("[%d]: %s %d", id, msg, i)
			c <- msg
			time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
		}
	}()
	return c
}

func main() {
	c1 := boring(1, "boring!")
	c2 := boring(2, "boring!")
	c3 := boring(3, "boring!")
	for i := 0; i < 20; i++ {
		fmt.Printf("You say: %q\n", <-c1)
		fmt.Printf("You say: %q\n", <-c2)
		fmt.Printf("You say: %q\n", <-c3)
	}
	fmt.Println("You'r boring, I'am leaving")
}
