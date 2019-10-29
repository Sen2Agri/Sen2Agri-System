/*
 * Copyright (C) 2018 CS ROMANIA
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */

package org.esa.sen2agri.services;

import org.esa.sen2agri.commons.Config;
import org.esa.sen2agri.commons.ProcessingTopic;
import org.esa.sen2agri.db.ConfigurationKeys;
import org.springframework.stereotype.Component;
import ro.cs.tao.messaging.Message;
import ro.cs.tao.messaging.NotifiableComponent;
import ro.cs.tao.utils.mail.MailSender;

import java.net.InetAddress;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.stream.Collectors;

@Component("batchNotifier")
public class BatchNotifier extends NotifiableComponent {
    private Timer timer;
    private long interval;

    public BatchNotifier() {
        super();
    }

    public long[] initialize() {
        int size = Integer.parseInt(Config.getSetting(ConfigurationKeys.MAIL_BATCH_LIMIT, "0"));
        if (size != 0) {
            this.queueSize = size;
        }
        this.interval = 60 * 60 * 1000;
        initTimer();
        return new long[] { size, interval };
    }

    @Override
    protected String[] topics() {
        return new String[] {
                ProcessingTopic.PROCESSING_COMPLETED.value(), ProcessingTopic.PROCESSING_WARNING.value(),
                ProcessingTopic.PROCESSING_ERROR.value(), ProcessingTopic.PROCESSING_ATTENTION.value()};
    }

    @Override
    protected void onMessageReceived(Message message) {
        synchronized(this.messageQueue) {
            this.messageQueue.offer(message);
            if (this.queueSize > 0 && this.messageQueue.size() == this.queueSize) {
                sendMail("Notification Summary", getLastMessages());
                initTimer();
            } else {
                if (ProcessingTopic.PROCESSING_ATTENTION.isParentOf(message.getTopic())) {
                    sendMail("Attention Needed", new ArrayList<Message>() {{ add(message); }});
                }
            }

        }
    }

    private void initTimer() {
        if (this.timer != null) {
            this.timer.cancel();
        }
        this.timer = new Timer("Batch reporter", true);
        this.timer.scheduleAtFixedRate(new TimedJob(), this.interval, this.interval);
    }

    private void sendMail(String subject, final List<Message> messages) {
        if (messages == null || messages.size() == 0) {
            logger.info("No messages to send");
            return;
        }
        try {
            MailSender mailSender = createSender();
            String subj = "[" + InetAddress.getLocalHost().getHostName() + "] " + subject;
            StringBuilder messageBuilder = new StringBuilder();
            Map<String, List<Message>> topicMessages = messages.stream().collect(Collectors.groupingBy(Message::getTopic));
            char[] chars;
            for (Map.Entry<String, List<Message>> entry : topicMessages.entrySet()) {
                List<Message> messageList = entry.getValue();
                messageList.sort(Comparator.comparingLong(Message::getTimestamp));
                chars = new char[entry.getKey().length()];
                Arrays.fill(chars, '=');
                messageBuilder.append(entry.getKey()).append("\n").append(new String(chars)).append("\n");
                for (Message message : messages) {
                    messageBuilder.append("Source:\t").append(message.getItem(Message.SOURCE_KEY)).append("\n");
                    LocalDateTime time = Instant.ofEpochMilli(message.getTimestamp())
                            .atZone(ZoneId.systemDefault()).toLocalDateTime();
                    messageBuilder.append("Time:\t").append(time.format(DateTimeFormatter.ISO_DATE_TIME)).append("\n");
                    messageBuilder.append(message.getItem(Message.PAYLOAD_KEY)).append("\n");
                    chars = new char[20];
                    Arrays.fill(chars, '-');
                    messageBuilder.append(new String (chars)).append("\n");
                }
            }
            mailSender.sendMail(subj, messageBuilder.toString());
        } catch (Throwable e) {
            logger.warning("Cannot send email notification. Reason: " + e.getMessage());
        }
    }

    private MailSender createSender() {
        String mailHost = Config.getSetting(ConfigurationKeys.MAIL_HOST, "smtp.gmail.com");
        int mailPort = Integer.parseInt(Config.getSetting(ConfigurationKeys.MAIL_PORT, "587"));
        String mailUser = Config.getSetting(ConfigurationKeys.MAIL_USER, "sen2agri.system@gmail.com");
        String mailPwd = Config.getSetting(ConfigurationKeys.MAIL_PASSWORD, "esa-sen2agri2019");
        String mailFrom = Config.getSetting(ConfigurationKeys.MAIL_SENDER, "sen2agri.system@gmail.com");
        String mailTo = Config.getSetting(ConfigurationKeys.MAIL_RECIPIENT, "");
        if (mailTo == null || mailTo.trim().isEmpty()) {
            throw new RuntimeException("Mail sender not configured");
        }
        boolean authRequired = Boolean.parseBoolean(Config.getSetting(ConfigurationKeys.MAIL_AUTH, "true"));
        boolean startTLS = Boolean.parseBoolean(Config.getSetting(ConfigurationKeys.MAIL_STARTTLS, "true"));
        return new MailSender(mailHost, mailPort, authRequired, mailUser, mailPwd, startTLS, mailFrom, mailTo);
    }

    private class TimedJob extends TimerTask {
        @Override
        public void run() {
            sendMail("Notification Summary", getLastMessages());
        }
    }
}
