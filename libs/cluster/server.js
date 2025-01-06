const { channel } = require('worker');
const tcp = require('tcp');

channel.on('message', (msg) => {
    if (msg.type !== 'SEND_FD') {
        return;
    }
    channel.postMessage({
        type: 'ACK_FD',
        fd: msg.fd,
    });
    const socket = new tcp.ServerSocket({fd: +msg.fd});
    require(msg.handler)(socket);
});
