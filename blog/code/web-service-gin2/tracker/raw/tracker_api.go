package tracker

import (
	"errors"
	"fmt"
	"lyfjaberg/rwe"
	"net"
	"net/url"

	"github.com/gin-gonic/gin"
)

func createTrack(c *gin.Context) {
	ctx := c.Request.Context()

	var in struct {
		Trackin *Trackin `json:"track"`
	}

	if err := c.BindJSON(&in); err != nil {
		fmt.Println(err)
		return
	}
	if in.Trackin == nil {
		c.Error(errors.New(`JSON field "track" is required`))
		return
	}

	var track Track

	if err := c.ShouldBindHeader(&track); err != nil {
		c.JSON(200, err)
	}

	track.IP = net.ParseIP(c.ClientIP())

	for _, ig := range rwe.GetCfg().IgnoredIP {
		if track.IP.Equal(ig) {
			fmt.Println("The track is ignored.")
			c.JSON(200, gin.H{"msg": ""})
		}
	}

	u, err := url.Parse(track.Referer)
	if err != nil {
		fmt.Println("Not worth tracking.")
		c.JSON(200, gin.H{"msg": ""})
	}

	host, _, _ := net.SplitHostPort(u.Host)
	addr := net.ParseIP(host)
	if addr != nil {
		if addr.IsPrivate() {
			fmt.Println("Hello, debugger!")
			c.JSON(200, gin.H{"msg": ""})
		}
	}

	track.CreatedAt = rwe.Clock.Now()
	track.FromUrl = in.Trackin.FromUrl
	track.Method = "POST"

	fmt.Println(track)

	if _, err := rwe.GetDB().
		ModelContext(ctx, &track).
		Insert(); err != nil {
		c.Error(err)
		return
	}

	c.JSON(200, gin.H{"msg": ""})
}

func createTrackFromGet(c *gin.Context) {
	ctx := c.Request.Context()

	var track Track

	if err := c.ShouldBindHeader(&track); err != nil {
		c.JSON(200, err)
	}

	track.IP = net.ParseIP(c.ClientIP())

	for _, ig := range rwe.GetCfg().IgnoredIP {
		if track.IP.Equal(ig) {
			fmt.Println("The track is ignored.")
			c.JSON(200, gin.H{"msg": ""})
		}
	}

	u, err := url.Parse(track.Referer)
	if err != nil {
		fmt.Println("Not worth tracking.")
		c.JSON(200, gin.H{"msg": ""})
	}

	host, _, _ := net.SplitHostPort(u.Host)
	addr := net.ParseIP(host)
	if addr != nil {
		if addr.IsPrivate() {
			fmt.Println("Hello, debugger!")
			c.JSON(200, gin.H{"msg": ""})
		}
	}

	track.CreatedAt = rwe.Clock.Now()
	track.Method = "Get"
	track.FromUrl = "NULL"

	fmt.Println(track)

	if _, err := rwe.GetDB().
		ModelContext(ctx, &track).
		Insert(); err != nil {
		c.Error(err)
		return
	}

	c.File("3.png")
}
