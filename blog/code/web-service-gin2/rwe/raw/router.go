package rwe

import (
	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
)

var (
	Router *gin.Engine
)

func init() {
	gin.SetMode(gin.ReleaseMode)
	Router = gin.Default()
	Router.Use(cors.Default())
}
