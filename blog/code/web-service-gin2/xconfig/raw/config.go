
package xconfig

import (
	"io/ioutil"
	"net"
	"os"
	"path/filepath"

	"gopkg.in/yaml.v2"
)

type Config struct {
	Postgres_url string   `yaml:"postgres_url"`
	IgnoredIP    []net.IP `yaml: ignored_ip`
}

func LoadConfig() (*Config, error) {
	config_file, err := filepath.Abs("dev.yml")
	if err != nil {
		return nil, err
	}

	f, err := os.Open(config_file)
	if err != nil {
		return nil, err
	}

	b, err := ioutil.ReadAll(f)
	if err != nil {
		return nil, err
	}

	cfg, err := parseConfig(b)
	if err != nil {
		return nil, err
	}

	return cfg, nil
}

func parseConfig(b []byte) (*Config, error) {
	cfg := new(Config)
	err := yaml.Unmarshal(b, cfg)
	if err != nil {
		return nil, err
	}
	return cfg, nil
}
