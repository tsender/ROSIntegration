#pragma once
#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include "libwebsockets.h"
THIRD_PARTY_INCLUDES_END

/** Buffer for one outgoing packet */
struct FLwsSendBuffer
{
	/**
	 * Constructor
	 * @param Data pointer to data to fill our buffer with
	 * @param Size size of Data
	 * @param bInIsBinary Whether or not this should be treated as a binary packet
	 */
	FLwsSendBuffer(const uint8* Data, const SIZE_T Size, const bool bInIsBinary);

	/** 
	 * Get the actual payload size
	 * Payload includes additional room for libwebsockets to use
	 * @return payload size
	 */
	int32 GetPayloadSize() const;

	/**
	 * Whether we have written our entire payload yet or not
	 * @return whether we have written our entire payload yet or not
	 */
	bool IsDone() const { return !HasError() && BytesWritten >= GetPayloadSize(); }

	/**
	 * Whether we have encountered an error or not
	 * @return whether we have encountered an error or not
	 */
	bool HasError() const { return bHasError; }

	/** Whether or not the packet is a binary packet, if not it is treated as a string */
	const bool bIsBinary;
	/** Number of bytes from Payload already written */
	int32 BytesWritten;
	/** Payload of the packet */
	TArray<uint8> Payload;
	/** Has an error occurred while writing? */
	bool bHasError;
};

/** Buffer for one incoming binary packet fragment */
struct FLwsReceiveBufferBinary
{
	/**
	 * Constructor
	 * @param Data pointer to data to fill our buffer with
	 * @param Size size of Data
	 * @param InBytesRemaining Number of bytes remaining in the packet
	 */
	FLwsReceiveBufferBinary(const uint8* Data, const int32 Size, const int32 InBytesRemaining);

	/** Payload received */
	TArray<uint8> Payload;
	/** Number of bytes remaining in the packet */
	const int32 BytesRemaining;
};

typedef TUniquePtr<FLwsReceiveBufferBinary> FLwsReceiveBufferBinaryPtr;

/** Buffer for one incoming text packet, fully received */
struct FLwsReceiveBufferText
{
	/**
	 * Constructor
	 * @param InText The packet contents
	 */
	FLwsReceiveBufferText(FString&& InText);

	/** Text packet received */
	const FString Text;
};

typedef TUniquePtr<FLwsReceiveBufferText> FLwsReceiveBufferTextPtr;