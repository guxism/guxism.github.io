
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
			fmt.Println(fmt.Sprintf("[%d]: ACK received", id))
		}
	}()
	return c
}

func fanIn(cs ...<-chan Message) <-chan Message {
	c := make(chan Message)
	n := len(cs)
	go func() {
		for i := 0; ; i++ {
			id := i % n
			c <- <-cs[id]
			time.Sleep(time.Duration(rand.Intn(1e3)) * time.Millisecond)
		}
	}()
	return c
}

func main() {
	var chans [5]<-chan Message
	for i := 0; i < 5; i++ {
		chans[i] = boring(i)
	}

	c := fanIn(chans[:]...)

	for {
		select {
		case msg := <-c:
			fmt.Printf("You say: %q\n", msg.str)
			msg.wait <- true
		case <-time.After(880 * time.Millisecond):
			fmt.Println("Timeout! You're too slow.")
			return
		}
	}
}

