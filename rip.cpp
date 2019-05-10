#include "rip.hpp"
#include <tins/udp.h>
#include <tins/ip.h>
#include <tins/ethernetII.h>
#include <tins/rawpdu.h>
#include <tins/ip_address.h>
#include "ip_helpers.hpp"



uint16_t co(uint8_t b)
{
    uint16_t result = 0;
    while (b > 0){
        result = result + (b & 1);
        b = b >> 1;
    }
    return result;
}

RipUpdate::RipUpdate(const Tins::RawPDU::payload_type& stream, std::string origin,Tins::IPv4Address orig_ip, ushort baseindex):
    metric_(stream[baseindex+19]),
    origin_intf_(origin),
    origin_ip_(orig_ip)
{

    uint32_t ip = stream[baseindex+7];
    ip = (ip << 8) + stream[baseindex+6];
    ip = (ip << 8) + stream[baseindex+5];
    ip = (ip << 8) + stream[baseindex+4];

    dst_prefix_.first = Tins::IPv4Address(ip);
    dst_prefix_.second = co(stream[baseindex+11]) + co(stream[baseindex+10]) + co(stream[baseindex+9]) + co(stream[baseindex+8]);


    // next hop si chces natahat ak to nebudu nuly, a pouzit ako origin
}


void Rip::SetIpObjs(InterfaceIP * i)
{
    intf_ip_ = i;
    for (const auto& entry: (*intf_ip_).data()){
        is_on_.insert({entry.first, false});
    }
}

void Rip::WillToggleRip(bool toggle, std::string intf)
{

    if (is_on_.find(intf) == is_on_.end()){
        assert(!"Rip::WillToggleRip call got nonexistent intf");
    }
    bool old  = is_on_[intf] ;
    is_on_[intf] = toggle;

    if (old == toggle){
        return;
    }

    auto iinfo = intf_ip_->data().at(intf);

    if (old == true && toggle == false) { // vypnutie
        auto oldentry = std::find_if(
                    db_locals_.begin(), db_locals_.end(),
                    [&intf](const RipUpdate& a){return a.origin_intf_ == intf; }
        );

        if (oldentry != db_locals_.end()){
            SendPoisionUpdate(*oldentry);
            db_locals_.erase(oldentry);

        }
        purge_intf(intf);
        return;
    }
    if (old == false && toggle == true) { //zapnutie
        auto update = RipUpdate(
                    iinfo.ip_ & Tins::IPv4Address::from_prefix_length(iinfo.pref_l_),
                    iinfo.pref_l_,
                    intf
        );
        db_locals_.push_back(update);
        SendPeriodicUpdate();
        return;
    }

    assert(!"all cases in Rip::WillToggleRip should be handled");
}
void Rip::SendPoisionUpdate(RipUpdate up, std::string s)
{
    up.origin_intf_ = s;
    SendPoisionUpdate(up);
}

void Rip::SendPoisionUpdate(RipUpdate up)
{
    up.metric_ = 16;
    SendTriggeredUpdate(up);
}

void Rip::SendTriggeredUpdate(RipUpdate up)
{
    // prebehni interfejsy okrem origin a poposielaj z nich kek
    for (const auto& entry: intf_ip_->data()){
        const auto& intf = entry.first;
        if (intf == up.origin_intf_ ||
            false == is_on_[entry.first] ||
            entry.second.ip_ == "0.0.0.0"
        ){
            continue;
        }
        if (up.metric_ < 16) up.metric_++;
        auto paket = Traffic(
                    Tins::IP("224.0.0.9", intf_ip_->data().at(intf).ip_) /
                    up.triggered_update(),
                    intf
        );
        emit DoSendRip(paket);
    }


}
void RipUpdate::serialize_to(Tins::RawPDU::payload_type& result) const
{
    const auto& u = *this;
    result.insert(result.end(),{0,2,0,0});
    auto ip = uint32_t(u.dst_prefix_.first);
    result.push_back((ip << 24) >> 24);
    result.push_back((ip << 16) >> 24);
    result.push_back((ip << 8) >> 24);
    result.push_back(ip >> 24);
    ip = uint32_t(Tins::IPv4Address::from_prefix_length(u.dst_prefix_.second));
    result.push_back((ip << 24) >> 24);
    result.push_back((ip << 16) >> 24);
    result.push_back((ip << 8) >> 24);
    result.push_back(ip >> 24);
    result.insert(result.end(),{0,0,0,0,0,0,0});
    result.push_back(uint8_t(u.metric_));

}

Tins::UDP RipUpdate::triggered_update() const
{
    Tins::RawPDU::payload_type result{2,2,0,0};

    serialize_to(result);

    return Tins::UDP(520,520) / Tins::RawPDU(result);
}

void Rip::SetIP(std::string intf, IPInfo)
{
    if (is_on_[intf]){
        WillToggleRip(false, intf);
        WillToggleRip(true, intf);
    }
}


void Rip::ProcessUpdate(const Traffic& t)
{
    const auto& iinfo = intf_ip_->data().at(t.in_intf_);
    if (is_on_[t.in_intf_] == false  ||
        iinfo.ip_ == "0.0.0.0" ||
        !match_prefix(
            t.frame_->rfind_pdu<Tins::IP>().src_addr(),
            PrefixInfo(iinfo.ip_ & Tins::IPv4Address::from_prefix_length(iinfo.pref_l_),iinfo.pref_l_)
        )
    ){
        return;
    }

    auto eh = t.frame_->find_pdu<Tins::UDP>();
    if (eh == nullptr){
        return;
    }

    if (eh->dport() != 520 || eh->sport() != 520 ){
        return;
    }

    auto s = eh->length();
    s = s - 12;

    auto pl = eh->find_pdu<Tins::RawPDU>()->payload();
    if (pl[1] != 2){
        return;
    }

    if ( s % 20 != 0 && pl.size() != s){
        return;
    }

    for (ushort index = 0 ; index < s ; index += 20){
        insert_update(RipUpdate(pl, t.in_intf_,t.frame_->rfind_pdu<Tins::IP>().src_addr(), index + 4 ));
    }

}

bool RipUpdate::operator==(const RipUpdate& o)
{
    return dst_prefix_ == o.dst_prefix_ &&
            metric_ == o.metric_ &&
            origin_intf_ == o.origin_intf_ &&
            origin_ip_  == o.origin_ip_;
}

void RipUpdate::reset_timers()
{
    flush_timer_ = RIP_FLUSH_TIMER;
//    holddown_timer_ = RIP_HOLDDOWN_TIMER;
    invalid_timer_ = RIP_INVALID_TIMER;
}

const std::vector<RipUpdate >& Rip::getDataBase() const
{
    return db_remotes_;
}

ForwardEntry RipUpdate::to_fwentry() const
{
    auto ei = ExitInfo(origin_intf_, origin_ip_, RIP);
    ei.weight_ = metric_;
    return { PrefixInfo(dst_prefix_.first, dst_prefix_.second), ei };
}


void Rip::insert_update(RipUpdate&& r)
{
    // ignoruj update o svojej vlastnej sieti
    if (std::find_if(
            db_locals_.begin(), db_locals_.end(),
            [&r](const RipUpdate& a){ return a.dst_prefix_ == r.dst_prefix_; }
    ) != db_locals_.end()  ){
        return ;
    }
    auto res = std::find(db_remotes_.begin(), db_remotes_.end(), r);
    if (res == db_remotes_.end()){
        res = std::find_if(
                db_remotes_.begin(), db_remotes_.end(),
                [&r](const RipUpdate& a){ return a.dst_prefix_ == r.dst_prefix_ ; }
        );
        if ( res == db_remotes_.end() ){
        // 1. vobec este nemam taky prefix - pushback and trigger
            db_remotes_.push_back(r);
            SendTriggeredUpdate(r);
            emit RipDatabaseChanged();
            return ;
        } else {
            // zaznam je v holddown tak ignor
            if ((*res).state_ != OK){
                return;
            }
        }

        if ( (*res).metric_ != r.metric_ ) {
        // 2. prisiel s lepsou/horsou metrikou - pushback and trigger
            if ((*res).metric_ > r.metric_){
                (*res) = r;
                emit RipDatabaseChanged();
                SendTriggeredUpdate(r);
            }
            if ( r.metric_ == 16 ) {
                (*res).metric_ = 16;
                (*res).state_ = RipUpdateState::POSSIBLY_DOWN;
                (*res).holddown_timer_ = RIP_HOLDDOWN_TIMER;
                emit RipDatabaseChanged();
                SendPoisionUpdate(*res, "null");
            }
            return;
        }
        if ( (*res).origin_ip_ != r.origin_ip_ || (*res).origin_intf_ != r.origin_intf_){
        // 3. prisiel od ineho zdroja - idk what to do, nothing i guess
            return;
        }

        assert(!"unhandled state at Rip::insert_update");
    } else {
        (*res).reset_timers();
    }

}

void Rip::purge_intf(std::string intf)
{
    db_remotes_.erase(
        std::remove_if(
            db_remotes_.begin(),
            db_remotes_.end(),
            [&intf](const RipUpdate& x){
                return x.origin_intf_ == intf;
            }
        ),
        db_remotes_.end()
    );
    emit RipDatabaseChanged();
}

void Rip::SendPeriodicUpdate()
{
    for (const auto& entry: intf_ip_->data()){
        const auto& intf = entry.first;
        const auto& intf_ip = entry.second.ip_;
        const auto& pred = [&intf](const RipUpdate& a){return a.origin_intf_ != intf;};

        if (false == is_on_[intf] || intf_ip == "0.0.0.0"){
            return;
        }

        std::vector<RipUpdate > for_intf;
        std::copy_if(
            db_remotes_.begin(),
            db_remotes_.end(),
            std::back_inserter(for_intf),
            pred
        );
        std::copy_if(
            db_locals_.begin(),
            db_locals_.end(),
            std::back_inserter(for_intf),
            pred
        );
        if (for_intf.empty()){
            return;
        }
        std::for_each(
            for_intf.begin(),
            for_intf.end(),
            [](RipUpdate &a){
                if(a.metric_< 16) a.metric_ ++;
                if(a.state_ != OK) a.metric_ = 16;
            }
        );
        Tins::RawPDU::payload_type payload{2,2,0,0};
        for (const auto& ru: for_intf){
            ru.serialize_to(payload);
        }
        auto paket = Traffic(
                    Tins::IP("224.0.0.9", intf_ip) /
                    Tins::UDP(520,520) / Tins::RawPDU(std::move(payload)),
                    intf
        );
        emit DoSendRip(paket);
    }
}

void RipUpdate::decrement_timers()
{
    if (invalid_timer_ > 0){
        invalid_timer_-- ;
    }
    if (holddown_timer_ > 0){
        holddown_timer_ -- ;
    }
    if (flush_timer_ > 0){
        flush_timer_ -- ;
    }
}



void Rip::UpdateLife()
{
    for (auto & entry: db_remotes_){
        entry.decrement_timers();
        if (entry.state_ == OK && entry.invalid_timer_ == 0){
            entry.state_ = HOLDDOWN;
            entry.holddown_timer_ = RIP_HOLDDOWN_TIMER;
            SendPoisionUpdate(entry);
        }
    }
    auto old_size = db_remotes_.size();
    if (!db_locals_.empty()){
        emit RipDatabaseTimersChanged();
    }

    db_remotes_.erase(
        std::remove_if(
            db_remotes_.begin(), db_remotes_.end(),
            [](const RipUpdate& r){ return r.flush_timer_ == 0 && r.state_ == HOLDDOWN ;}
        ),
        db_remotes_.end()
    );
    if ( old_size != db_remotes_.size()){
        emit RipDatabaseChanged();
    }
}

Rip::Rip(QObject *parent) : QObject(parent)
{
    QObject::connect(&update_timer_, SIGNAL(timeout()), this, SLOT(SendPeriodicUpdate()));
    update_timer_.start(30000);

    QObject::connect(&clock_, SIGNAL(timeout()), this, SLOT(UpdateLife()));
    clock_.start(1000);
}
