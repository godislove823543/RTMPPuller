# RTMPPuller
從 OBS 推流到 SRS Server，再從 SRS 的 RTMP 協定去拉流，拉流端建立連線，客戶端發送命令訊息包含 set chunk size、release stream、create stream、check bandwidth、play 等操作，完成後伺服器會開始回傳各種 RTMP Message，包括 控制訊息（window acknowledgement、set peer bandwidth）、事件訊息（stream begin）以及資料訊息（ScriptData / onMetaData)，過程中將接收到的訊息陸續做解析。
