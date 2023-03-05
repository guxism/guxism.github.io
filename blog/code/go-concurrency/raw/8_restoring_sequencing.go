
package main

import (
	"fmt"
	"math/rand"
	"time"
)

type Message struct {
	str  string
	wait chan bool
}

func boring(id int) <-chan Message {
	c := make(chan Message)
	waitForIt := make(chan bool)
	go func() {
		for i := 0; ; i++ {
			c <- Message{
				str:  fmt.Sprintf("[%d] %d", id, i),
				wait: waitForIt,
			}
			time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
			<-waitForIt
		}
	}()
	return c
}

func fanIn(cs ...<-chan Message) <-chan Message {
	c := make(chan Message)
	for i := range cs {
		go func(id int) {
			input := cs[id]
			for {
				c <- <-input
				time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
			}
		}(i)
	}
	return c
}

func main() {
	var chans [5]<-chan Message
	for i := 0; i < 5; i++ {
		chans[i] = boring(i)
	}

	c := fanIn(chans[:]...)
	for i := 0; i < 5; i++ {
		for i := 0; i < 5; i++ {
			msg := <-c
			fmt.Printf("You say: %q\n", msg.str)
			msg.wait <- true
		}
	}
	fmt.Println("You'r boring, I'am leaving")
}

