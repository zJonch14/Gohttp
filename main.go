package main

import (
    "fmt"
    "math/rand"
    "net"
    "os"
    "strconv"
    "sync"
    "time"
)

func main() {
    if len(os.Args) < 5 {
        fmt.Println("Uso: go run main.go <ip> <port> <cantidad> <tiempo_segundos>")
        return
    }
    ip := os.Args[1]
    port, _ := strconv.Atoi(os.Args[2])
    cantidad, _ := strconv.Atoi(os.Args[3])
    tiempo, _ := strconv.Atoi(os.Args[4])

    var wg sync.WaitGroup
    addr := fmt.Sprintf("%s:%d", ip, port)
    timeout := time.After(time.Duration(tiempo) * time.Second)

    fmt.Printf("Iniciando %d conexiones 'simuladas' UDP a %s por %d segundos...\n", cantidad, addr, tiempo)

    for i := 0; i < cantidad; i++ {
        wg.Add(1)
        go func(idx int) {
            defer wg.Done()
            conn, err := net.Dial("udp", addr)
            if err != nil {
                fmt.Printf("[#%d] Error: %v\n", idx, err)
                return
            }
            defer conn.Close()
            // Genera datos aleatorios para enviar
            payload := make([]byte, 16)
            rand.Seed(time.Now().UnixNano() + int64(idx))
            rand.Read(payload)
            for {
                select {
                case <-timeout:
                    // Al terminar el tiempo, dejamos de enviar paquetes
                    return
                default:
                    // Envía paquete cada 2 segundos (puedes cambiar el tiempo)
                    conn.Write(payload)
                    time.Sleep(2 * time.Second)
                }
            }
        }(i)
        time.Sleep(10 * time.Millisecond) // Delay opcional para evitar picos
    }

    wg.Wait()
    fmt.Println("Prueba terminada, todas las conexiones 'cerradas'.")
}
