
// go get -u github.com/panjf2000/ants - не понадобился, но похоже неплох
// go get -u github.com/goinggo/workpool
// go run go_bench.go

package main

import (
    "fmt"
    "time"
    "runtime"
    // "container/list"
    "sync"
    "github.com/goinggo/workpool"
)

type GrabbedFrameDescriptor struct {
    frame_idx int
    contents []int
    capturingtime time.Time
    recog_result int
}

type MyWork struct {
    frame GrabbedFrameDescriptor
    WP *workpool.WorkPool
    c chan GrabbedFrameDescriptor
}

func (mw *MyWork) DoWork(workRoutine int) {
    // Real work
    // fmt.Printf("%s : %d\n", mw.Name, mw.BirthYear)
    // fmt.Printf("Q:%d R:%d\n", mw.WP.QueuedWork(), mw.WP.ActiveRoutines())

    // Simulate some delay
    // time.Sleep(500 * time.Millisecond)
    mw.c <- mw.frame
}

type CyclicalBuffer struct {
    q []GrabbedFrameDescriptor
    rd_wr_ptr int
    size int
}

func (buffer *CyclicalBuffer) PushBack(frame GrabbedFrameDescriptor) GrabbedFrameDescriptor {
    oldest_frame := buffer.q[buffer.rd_wr_ptr]
    buffer.q[buffer.rd_wr_ptr] = frame
    buffer.rd_wr_ptr = (buffer.rd_wr_ptr+1) % buffer.size
    return oldest_frame
}

func (buffer *CyclicalBuffer) SearchByFrameIdx(frame_idx int) int {
    // fmt.Printf("VQ: ")
    // for i := 0; i < buffer.size; i++ {
    //     fmt.Printf(" %d", buffer.q[i].frame_idx)
    // }
    // fmt.Printf("\n")

    for i := 0; i < buffer.size; i++ {
        if (buffer.q[i].frame_idx == frame_idx) {
            return i
        }
    }
    return -1
}


func main() {   
    // Name
    // "Реальтаймная/Онлайн система обработки данных с модели камеры"
    // Компоненты - трейдпулы, очереди задачи(потокобезопансые)
    
    // Требования для автопилота по камере "Теслы".
    // Фиксированная latence и максимальная fps при заданных ресурсах
    // 1. High frame-rate
    // 2. Real-Time/Latency - нужна актуальная информация - входная очередь и скипы фреймов 
    // Время между впавшим в обработу и выпавшим минимальное - lantancey
    // Эксперимент
    // Меняем количество задейстовванного железа и смотрим на FPS и Latency, 
    // Закон Адмала.
    // 1. Архитектура и инфрастурктуря для запуска
    //   https://medium.com/statuscode/pipeline-patterns-in-go-a37bb3a7e61d
    //   https://stackoverflow.com/questions/15715605/multiple-goroutines-listening-on-one-channel
    // 2. в компонентах потюнить параемтер, сколько ядрер, какая длина очередей, количество 
    //   пропусков.

    // Params
    var threads_count int32 = 5
    allowed_latency_frames := 7
    var ds_ms time.Duration = 500  // 40  

    // TEST
    // k:=10
    // j := 1
    // for k > 0 {
    //     frame := GrabbedFrameDescriptor {
    //         frame_idx: j,
    //         capturingtime: time.Now(),
    //         recog_result: 0,
    //     }
    //     q.PushBack(frame)
    //     queue_position := q.SearchByFrameIdx(frame.frame_idx)
    //     fmt.Printf(" Q:%d %d\n", frame.frame_idx, q.q[queue_position].frame_idx)
    //     k -= 1
    //     j += 1
    // }
    // return

    // Worker
    // https://dev.to/panjf2000/releasing-a-high-performance-goroutine-pool-in-go-n57
    // https://www.ardanlabs.com/blog/2013/05/thread-pooling-in-go-programming.html
    runtime.GOMAXPROCS(runtime.NumCPU())
    workPool := workpool.New(runtime.NumCPU(), threads_count)
    defer workPool.Shutdown("routine")

    // Main queue - thread-safe should be
    // FIXME: Нужно будет искать по очереди нужный фрейм - потокобезопасно
    // https://yourbasic.org/golang/implement-fifo-queue/
    // https://stackoverflow.com/questions/2818852/is-there-a-queue-implementation  
    // https://golang.org/pkg/container/list/
    lock := sync.Mutex{}
    //queue := list.New()  // лучше иметь одну на все обработчики
    // New queue
    queue := CyclicalBuffer{
        q : make([]GrabbedFrameDescriptor, allowed_latency_frames),
        rd_wr_ptr : 0,        
        size : allowed_latency_frames,
    }

    // Performans
    // https://blog.golang.org/maps
    perf_lock := sync.Mutex{}
    // var m map[string]int
    // m = make(map[string]int)
    
    // Load
    grabber_interrupt := time.NewTicker(ds_ms * time.Millisecond)
    quit_grabber_interrupt := make(chan struct{})
    
    rnn_ch := make(chan GrabbedFrameDescriptor, 1)
    recog_ch := make(chan GrabbedFrameDescriptor, 1)
    go func() {
        // Читаем из файла фреймы или из граббера и шлем остальным модулям
        // на обработку
        global_frame_idx := 0
        for {
           select {
            case <- grabber_interrupt.C:     
                global_frame_idx += 1

                // load image from file?
                // https://medium.com/mop-developers/image-processing-in-go-5ba9a9043bc2
                // 2D array
                // twoD := make([][]int, 3)
                // for i := 0; i < 3; i++ {
                //     innerLen := i + 1
                //     twoD[i] = make([]int, innerLen)
                //     for j := 0; j < innerLen; j++ {
                //         twoD[i][j] = i + j
                //     }
                // }
                frame := GrabbedFrameDescriptor {
                    frame_idx: global_frame_idx,
                    capturingtime: time.Now(),
                    recog_result: 0,
                }

                // Attention!!! Видимо везде копируем, хз как в Go сделать ссылки
                // Пушаем в основную очередь
                // {
                //     lock.Lock()
                //     queue.PushBack(frame)
                //     for queue.Len() > allowed_latency_frames {
                //         e := queue.Front() // First element
                //         // Sent to RNN
                //         rnn_ch <- e.Value.(GrabbedFrameDescriptor)  // особенность работы списка

                //         queue.Remove(e) // Dequeue
                //     }
                //     lock.Unlock()
                // }

                {
                    lock.Lock()
                    e := queue.PushBack(frame)
                    rnn_ch <- e // особенность работы списка
                    lock.Unlock()
                }

                // Шлем фрейм грабберу
                // В один канал двоим читателям не отправить 
                recog_ch <- frame

            case <- quit_grabber_interrupt:
                grabber_interrupt.Stop()
                return
            }
        }
     }()

    // Recog
    // максимум один в очереди - latency
    work_finish_ch := make(chan GrabbedFrameDescriptor, 1)  
    go func() {
        // Находим фрейм к которому нужно приписать результаты
        for {
            frame := <-recog_ch
            work := MyWork {
                frame: frame,
                WP: workPool,
                c: work_finish_ch,
            }
          
            if err := workPool.PostWork("routine", &work); err != nil {
                fmt.Printf("skip: %s\n", err)
            }
        }
    }()

    // Result acceptor - приписывает результаты к фреймам
    go func() {
        for {
            frame := <-work_finish_ch           
        
            lock.Lock() 

            fmt.Printf("Done frame_idx:%d\n", frame.frame_idx)
            queue_position := queue.SearchByFrameIdx(frame.frame_idx)
            if queue_position != -1 {
                queue.q[queue_position].recog_result = 1
            }

            
            // for e := queue.Front(); e != nil; e = e.Next() {
            //     // do something with e.Value
            //     current_frame_idx := e.Value.(GrabbedFrameDescriptor).frame_idx
            //     fmt.Printf(" Q:%d\n", current_frame_idx)
            //     if (current_frame_idx == frame.frame_idx) {
            //         // https://webapplicationconsultant.com/go-lang/cannot-assign-to-struct-field-in-map/
            //         // Can't assign
            //         // e.Value.(GrabbedFrameDescriptor).recog_result = 1
            //     }
            // }

            // Put to RNN queue
            lock.Unlock()           
        }
     }()

    // Fake Rnn - Использует результаты распознвания для трекинга пешеходов например
    // или дорожной разметки
    go func() {
        for {
            frame := <-rnn_ch           
            fmt.Printf("RNN got frame_idx:%d recog_result:%d\n", frame.frame_idx,
                frame.recog_result)            
        }
    }()

    // Performance
    perf_counter_interrupt := time.NewTicker(1 * time.Second)
    quit_perf_counter_interrupt := make(chan struct{})
    go func() {
        for {
           select {
            case <- perf_counter_interrupt.C:
                // do stuff
                perf_lock.Lock()
                // Достаем из мапы что есть, и сбрасываем
                fmt.Printf("PerfCouter:...\n")
                perf_lock.Unlock()

            case <- quit_perf_counter_interrupt:
                perf_counter_interrupt.Stop()
                return
            }
        }
    }()


    time.Sleep(50 * time.Second)
}


