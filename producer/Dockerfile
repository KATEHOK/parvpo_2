# Use the official Go image as the base image
FROM golang:latest

# Set the working directory inside the container
WORKDIR /app

# Copy the producer.go file to the working directory
COPY producer.go .

RUN export GO111MODULE=off

RUN go build -o producer producer.go

CMD ./producer $ProdNum $MSize
