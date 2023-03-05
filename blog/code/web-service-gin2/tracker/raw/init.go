package tracker

import (
	"lyfjaberg/rwe"
)

func init() {
	rwe.Router.POST("/tracks", createTrack)
	rwe.Router.GET("/tracks", createTrackFromGet)
}
