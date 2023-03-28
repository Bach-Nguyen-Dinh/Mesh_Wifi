# **Đề tài : Mô phỏng hệ thống Mesh Wifi**
<br/>
<br/>

## **1. Giới thiệu mạng mesh không dây**<br/>
### **1.1. Khái niệm**<br/>
Mạng mesh là một cấu trúc liên kết trong mạng LAN với một tập hợp các node giao tiếp với nhau. Mỗi node đều có khả năng kết nối tới các node còn lại một cách trực tiếp hoặc gián tiếp (thông qua các node trung gian đóng vai trò chuyển tiếp). Kết nối giữa các node không là cố định và vai trò của mỗi node đối với toàn bộ cấu trúc là như nhau. 

Tùy vào lập trình và các thuật toán sử dụng mà cấu trúc của mạng mesh sẽ tự động điều chỉnh trước những thay đổi về số lượng node trong mạng hoặc thay đổi về kết nối giữa các node.
<br/>
<p align="center">
  <img src="https://github.com/Bach-Nguyen-Dinh/Mesh_Wifi/blob/master/pictures/mesh_architecture.jpg"/>
</p>

### <br/>**1.2. Thành phần trong hệ thống**<br/>
Các thành phần mạng trong một mạng mesh không dây
- Relay / Repeater: có vai trò chuyển tiếp bản tin và nếu cần sẽ khuếch đại tín hiệu trong trường hợp bản tin có nguy cơ bị suy giảm chất lượng
- Gateway / Router: giao tiếp giữa mạng mesh nội bộ với các mạng khác và định tuyến cho thông tin đi giữa các mạng với nhau
- Access Point: cung cấp kết nối cho các thiết bị End device hoặc Client vào mạng
- End device / Client: thiết bị để người dùng tương tác với mạng
<br/>Tuy nhiên, với sự phát triển của công nghệ sản xuất IC, nhiều thành phần mạng đã được tích hợp chung với nhau. Thành phần có chức năng chuyển tiếp bản tin, định tuyến cho bản tin và cung cấp kết nối cho vào mạng được gọi là mesh router. Thành phần được sử dụng bởi người dùng để tăng tác với mạng gọi là mesh client. Mesh router cũng có thể bao gồm cả chức năng của gateway cho phép mạng mesh truy cập vào Internet.

Về lí thuyết, kết nối giữa các thành phần trong mạng mesh không dây có thể sử dụng bất kì công nghệ truyền thông bằng sóng radio hiện hành nào (3G/4G, Wifi, Bluetooth, …). Tuy nhiên, trong thực tế, đa số các trường hợp sử dụng công nghệ Wifi, cụ thể hơn là họ các giao thức IEEE 802 (IEE 802.11, IEE802.16, …). Lí do đơn giản bởi độ phổ biến của công nghệ Wifi đồng thời chi phí thiết bị đã trở nên bão hòa.

### <br/>**1.3. Thuật toán định tuyến**<br/>
Mạng mesh được đặc trưng bởi khả năng tự tổ chức và tự phục hồi, đồng thời có khả năng chuyển tiếp bản tin giữa các node trong hệ thống tạo nên một mạng lưới truyền thông liên tục không gián đoạn từ node nguồn tới node đích. Do đó, nhiều giao thức định tuyến được phát triển và điều chỉnh để phù hợp với tính chất của mạng mesh. 
<br/>
<p align="center">
  <img src="https://github.com/Bach-Nguyen-Dinh/Mesh_Wifi/blob/master/pictures/routing_method.jpg"/>
</p> 
Nguyên lí chung của các thuật toán định tuyến là mỗi node giao tiếp thông tin định tuyến của nó với các node khác trong mạng, ví dụ, các trường bổ sung thông tin trong một khung bản tin. Với thông tin nhận được, dựa trên chức năng của giao thức định tuyến, mỗi node sẽ quyết định chuyển tiếp hay giữ dữ liệu cho chính node đó. Điều cần thiết đối với bất kỳ thuật toán định tuyến nào là đảm bảo rằng việc định tuyến được thực hiện sẽ dự đoán đường đi ngắn nhất giữa node nguồn và node đích. Bất cứ khi nào có sự thay đổi trong cấu trúc mạng như việc thêm hay xóa các node sẽ yêu cầu phải cập nhật lại đường đi.<br/><br/>
Định tuyến chủ động - Proactive Routing - Các giao thức chủ động cố gắng đánh giá liên tục tất cả các tuyến trong mạng để khi một gói cần được chuyển tiếp, đường đi của bản tin đã được biết và sẵn sàng để sử dụng.
Ví dụ, Routing method, dữ liệu được truyền theo 1 đường, mạng phải có khả năng tự cấu hình lại khi một node bị hỏng hoặc bổ sung thêm node mới hoặc kết nối không ổn định bằng các thuật toán như Shortest Path Bridging hay TRILL (TRansparent Interconnection of Lots of Links)
  
<br/>Định tuyến thụ động - Reactive Routing - Các giao thức định tuyến thụ động sẽ gọi đến một chương trình con để tìm đường khi có yêu cầu. Phương pháp định tuyến thụ động thường hoạt động dựa trên mô hình truy vấn/trả lời, tất cả các node trong hệ thống sẽ nhận bản tin cho đến khi gửi tới đích đến mong muốn. 

Định tuyến kết hợp - Hybrid Routing - Các giao thức thuộc nhóm này này tận dụng cả kỹ thuật chủ động và bị động để xác định đường đi tốt nhất giữa bất kỳ cặp nút nào.

### <br/>**1.4. Các lợi thế của mạng mesh không dây**<br/>
- Độ bao phủ cao: đối với các loại mạng truyền thống, kết nối mạng được phân phối qua một điểm truy cập duy nhất, nó có nghĩa là nếu khu vực cách xa bộ định tuyến hoặc khu vực có nhiều vật cản che khuất đường truyền, kết nối sẽ bị chậm hoặc đứt đoạn.
- Độ ổn định: dựa vào các thuật toán định tuyến mà kết nối giữa các node với nhau sẽ gần như luôn được đảm bảo

<br/><br/>
## **2. Giới thiệu socket**<br/>
Các thiết bị trong một mạng giao tiếp với nhau thông qua một khải niệm là socket. Để tạo một socket cần các tham số:
-	Address family: sử dụng loại địa chỉ IPv4 hay loại địa chỉ Ipv6
-	Loại socket: stream socket hay datagram socket
-	Giao thức của socket: ứng với loại socket sử dụng giao thức TCP hay giao thức UDP
<br/>
Sau khi khởi tạo thành công một socket, thiết bị đó có thể thực hiện kết nối tới các thiết bị khác thông qua socket vừa tạo nhưng không thể lắng nghe các thiết bị khác kết nối tới.
<br/>
<p align="center">
  <img src="https://github.com/Bach-Nguyen-Dinh/Mesh_Wifi/blob/master/pictures/socket.jpg"/>
</p> 

Để có thể cho phép các thiết bị khác kết nối đến, cần phải tạo một socket mới. Socket này có nhiện vụ lắng nghe các kết nối tới. Đồng thời, socket mới này cần phải được gán với ít nhất một địa chỉ IP và một số port, hành động này gọi là bind. Với cùng một địa chỉ IP nhưng với các số port khác nhau, một thiết bị có thể thực hiện nhiều chức năng khác nhau. 

Trong mô hình client - server, client là thiết bị kết nối tới server, server là thiết bị lắng nghe kết nối từ client. Một quá trình giao tiếp đơn giản giữa client và server có thể được mô tả như sau:
 


<br/><br/>
## **3. Yêu cầu mô phỏng mạng mesh wifi**<br/>
### **3.1. Kịch bản 1**<br/>
Hệ thống gồm 4 node, trong trạng thái hoạt động bình thường có kết nối được mô tả như sau:
<br/>
<p align="center">
  <img src="https://github.com/Bach-Nguyen-Dinh/Mesh_Wifi/blob/master/pictures/scenario_1_layout.jpg"/>
</p> 
 
Từ node A có thể gửi bản tin trực tiếp đến cả 3 node còn lại.
Khi xảy ra sự cố mất kết nối giữa node A và node C, node A sẽ phải hỏi những node còn lại tìm một đường đi mới cho bản tin đến node C. Nếu có node nào phản hồi là tìm được đường thì gửi bản tin đến node C theo đường đó.
<br/><br/>
### **3.2. Kịch bản 2**<br/>
Hệ thống gồm 4 node, trong trạng thái hoạt động bình thường có kết nối được mô tả như sau:
<br/>
<p align="center">
  <img src="https://github.com/Bach-Nguyen-Dinh/Mesh_Wifi/blob/master/pictures/scenario_2_layout.jpg"/>
</p> 

Khi xảy ra sự cố mất kết nối giữa node A và node C, node A sẽ phải hỏi những node còn lại tìm một đường đi mới cho bản tin đến node C. Bản tin tìm đường sẽ phải thực hiện quá trình chuyển tiếp qua các node trung gian. Tại các node trung gian, chúng cũng tiếp tục phải hỏi các node xung quanh để chuyển tiếp bản tin bằng các node trung gian khác. 


<br/><br/>
## **4. Kịch bản mô phỏng**<br/>
Giả sử sau khi gửi bản tin broadcast, mỗi node sẽ có một danh sách về số lượng các node có thể kết nối tới và thông tin về địa chỉ IP và số port của các node đó. VD: node A đã biết địa chỉ IP và số port của các node B, node C và node D.

### **4.1. Trường hợp hoạt động bình thường**<br/>
Do node A biết có đường kết nối tới node C, vì vậy node A sẽ không hỏi các node B và node D tìm đường tới node C

 


Kết quả chạy thực tế
Thứ tự lần lượt node A và nodeC
<br/><br/>
### **4.2. Trường hợp mất kết nối**<br/>
**4.2.1. Kịch bản 1**<br/>
Do node A mất kết nối với C, vì vậy node A sẽ gửi bản tin tìm đường lần lượt tới những node còn lại. 

Các node đó lại lần lượt gửi bản tin tìm đường đến node C đến những node xung quanh. Quá trình lặp lại cho đến khi không node nào tìm thấy node C hoặc node C là một trong các node lân cận của một node đang được nhận bản tin tìm đường.

Trong quá trình hỏi lần lượt để tìm đường, mỗi lượt hỏi và đợi phản hồi sẽ có time out rồi mới chuyển sang hỏi node tiếp theo. 
 

Sau khi tìm thành công đường đi mới đến node C, các node trên đường đi sẽ chuyển sang chế độ đợi bản tin gửi đến để chuyển tiếp bản tin đó cho node C.

Nếu trong quá trình chuyển tiếp bản tin đến node C mà có bất kì node nào trong đường đi bị ngắt kết nối thì sẽ báo lỗi không thể gửi bản tin đến node C. Nếu muốn gửi lại thì phải lặp lại quá trình tìm đường đi đến node C và chuyển tiếp bản tin đến node C.
 


Kết quả chạy thực tế<br/>
Thứ tự lần lượt node A, node B và node C
 
<br/>**4.2.2. Kịch bản 2**<br/>
Trong trường hợp có nhiều node trung gian, quá trình tìm đường đến node C sẽ cần phải lặp lại quá trình chuyển tiếp bản tin tìm đường qua các node trung gian. 
 
Kết quả chạy thực tế<br/>
Thứ tự lần lượt là node A, node B, node D và node C

<br/><br/>
## **5. Các nguy cơ, độ an toàn của mạng mesh**<br/>
Với mạng mesh được lập trình như trên, nguy cơ đầu tiền là bản tin bị rò rỉ. Do quá trình tìm đường hoạt động theo nguyên lí gửi bản tin tìm đường cho toàn bộ các node đang hoạt động trong mạng. Nên một node hoàn toàn có thể định tuyến đường đi bản tin đến một node đích khác.

Tiếp theo, bản tin chưa có mã hóa nên các node trung gian hoàn toàn có thể đọc được nội dung của bản tin và thay đổi nội dung của bản tin gốc trước khi bản tin đến được node đích.

Hơn nữa, khung bản tin chưa có trường phát hiện và kiểm tra lỗi và cũng không có khả năng tự sửa lỗi. Do đó, ta không có cách nào để biết được bản tin nhận được có chính xác với bản gốc hay không.

Cuối cùng, khi đã tìm đường đường đi, bản tin chỉ gửi theo đúng đường vừa tìm được, nếu trong quá trình chuyển bản tin, một node trung gian bị ngắt khỏi hệ thống thì bản tin sẽ không thể được truyền tới node đích và sẽ báo về là gửi không thành công.
